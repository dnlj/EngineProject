// Game
#include <Game/systems/ActionSystem.hpp>
#include <Game/World.hpp>
#include <Game/comps/ActionComponent.hpp>


namespace {
	using namespace Game;
	using EstBuffSize = uint8;
	constexpr int32 tmax = std::numeric_limits<EstBuffSize>::max();
	constexpr int32 range = (tmax - ActionComponent::maxStates) / 2;

	constexpr EstBuffSize estBuffSizeToNet(const float32 v) noexcept {
		auto t = static_cast<int32>(v) + range;
		return std::max(0, std::min(t, tmax));
	};

	constexpr float32 estBuffSizeFromNet(EstBuffSize v) noexcept {
		return static_cast<float32>(v - range);
	};
}


namespace Game {
	ActionSystem::ActionSystem(SystemArg arg)
		: System{arg}
		, actionFilter{world.getFilterFor<ActionComponent>()} {
	}

	void ActionSystem::preTick() {
		for (const auto ent : actionFilter) {
			const auto tick = world.getTick();
			auto& actComp = world.getComponent<ActionComponent>(ent);
			if constexpr (ENGINE_CLIENT) {
				const auto& last = actComp.states.get(tick - 1);
				actComp.state = &actComp.states.insert(tick);
				*actComp.state = last;
				for (auto& btnState : actComp.state->buttons) {
					btnState.pressCount = 0;
					btnState.releaseCount = 0;
				}
			} else {
				actComp.state = actComp.states.find(world.getTick());
			}
		}
	}

	void ActionSystem::tick() {
		// TODO: On client - If server didnt get correct input we need to rollback and mirror that loss on our side or we will desync
		// TODO: dont resend actions when performing rollback
		const auto currTick = world.getTick();
		for (const auto ent : actionFilter) {
			auto& actComp = world.getComponent<ActionComponent>(ent);
			auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
			auto* state = actComp.state;

			if constexpr (ENGINE_CLIENT) {
				conn.msgBegin<MessageType::ACTION>();
				conn.write(currTick);

				// TODO: how many to send?
				for (auto t = currTick - (actComp.states.capacity() / 4); t <= currTick; ++t) {
					const auto& s = actComp.states.get(t);
					
					for (auto b : s.buttons) {
						conn.write<2>(b.pressCount);
						conn.write<2>(b.releaseCount);
						conn.write<1>(b.latest);
					}
					
					for (auto a : s.axes) { // TODO: compress
						conn.write<32>(reinterpret_cast<uint32&>(a));
					}
				}
				conn.writeFlushBits();
				conn.msgEnd<MessageType::ACTION>();
			} else if constexpr (ENGINE_SERVER) {
				conn.msgBegin<MessageType::ACTION>();
				conn.write(currTick);
				conn.write(state ? state->recvTick : 0);
				conn.write(estBuffSizeToNet(actComp.estBufferSize));
				conn.msgEnd<MessageType::ACTION>();

				if (!state) {
					ENGINE_LOG("Missing input for tick ", currTick);
					// TODO: duplicate and decay last input?
					actComp.state = &(actComp.states.insert(currTick) = {});
				}

				actComp.states.remove(currTick - 1);

				if constexpr (ENGINE_DEBUG) {
					if (world.hasComponent<NetworkStatsComponent>(ent)) {
						auto& netStatsComp = world.getComponent<NetworkStatsComponent>(ent);
						//netStatsComp.trend = actComp.tickTrend;
						netStatsComp.inputBufferSize = actComp.states.max() - actComp.states.minValid();
					}
				}

				//ENGINE_LOG("Buffer size: ",
				//	actComp.states.capacity(), " | ",
				//	actComp.states.max(), " ", actComp.states.minValid(), " = ",
				//	actComp.states.max() - actComp.states.minValid());
				// If we ever add lag compensation we will need to handle server rollback here.
			}
		}
	}

	void ActionSystem::recvActions(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		if constexpr (ENGINE_SERVER) {
			recvActionsServer(from, head, fromEnt);
		} else {
			recvActionsClient(from, head, fromEnt);
		}
	}

	void ActionSystem::recvActionsClient(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		// TODO: what about ordering?
		// TODO: we dont actually use tick/recvTick. just nw buffsize
		constexpr float32 maxStates = static_cast<float32>(decltype(ActionComponent::states)::capacity());
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto recvTick = *from.read<Engine::ECS::Tick>();
		const auto buffSize = tick - recvTick;

		auto& actComp = world.getComponent<ActionComponent>(fromEnt);
		{
			const auto est = estBuffSizeFromNet(*from.read<uint8>());

			// NOTE: seems to work fine without this.
			//float32 ping = std::chrono::duration<float32, std::milli>{from.getPing()}.count();
			//float32 ticksPerPing = Engine::Clock::Milliseconds{from.getPing()} / Engine::Clock::Milliseconds{World::getTickInterval()};
			//
			//// Interpolate between 0.2 and 0.01 based on ping
			//constexpr float32 m = (0.01f - 0.2f) / (maxStates - 1.0f);
			//constexpr float32 b = (-maxStates * m) + 0.01f;
			//float32 smoothing = m * ticksPerPing + b;

			//actComp.estBufferSize += (est - actComp.estBufferSize) * smoothing;
			actComp.estBufferSize = est;
		}

		// TODO: this is ideal buffer size not tick lead...
		const auto idealTickLead = [&](){
			constexpr auto tickDur = World::getTickInterval();
			const auto p2 = from.getPing().count() * 0.5f; // One way trip
			const auto j2 = static_cast<float32>(from.getJitter().count());
			const auto avgTripTime = p2 * (1.0f + from.getLoss()) + j2;
			const auto avgTicksPerTrip = std::ceil(avgTripTime / tickDur.count());
			// TODO: probably also want to track a stat of how many inputs we have missed in the last X seconds. (exp avg would be fine)
			return avgTicksPerTrip + 1;
		};

		// TODO: ideal should be smoothed over time
		const auto ideal = idealTickLead();

		// Used to adjust tickScale when based on trend. Scaled in range [trendAdjust, maxTrendScale + trendAdjust]
		constexpr float32 maxTickScale = 8.0f;
		constexpr float32 maxBufferSize = maxStates;

		float32 diff = actComp.estBufferSize - ideal;

		const auto calcTickScale = [&](const float32 diff){
			float32 scale = (maxTickScale/(range + maxStates));
			const auto ping = from.getPing();

			// Be more conservative at larger pings to prevent bouncing between too high and too low due to slower feedback
			if (ping > std::chrono::milliseconds{350}) {
				scale *= 0.25f;
			} else if (ping > std::chrono::milliseconds{250}) {
				scale *= 0.50f;
			} else if (ping > std::chrono::milliseconds{100}) {
				scale *= 0.75f;
			} else if (ping > std::chrono::milliseconds{50}) {
				scale *= 0.95f;
			}

			return 1.0f + (diff * scale);
		};

		// NOTE: seems to work fine without this
		// Prevent large changes from ping spikes
		//if (from.getJitter() > std::chrono::milliseconds{50}) {
		//	diff *= 0.25f;
		//}

		// Slow down when we are close to the correct value
		if (std::abs(diff) < ideal + 8.0f) {
			diff *= 0.75f;
		}

		// Determine the tick scale
		constexpr float32 eps = 0.8f;
		if (diff < -eps) {
			world.tickScale = 1.0f / calcTickScale(std::abs(diff));
			ENGINE_LOG("++scale ", diff, " ", world.tickScale);
		} else if (diff > eps) {
			world.tickScale = calcTickScale(diff);
			ENGINE_LOG("--scale ", diff, " ", world.tickScale);
		} else {
			world.tickScale = 1.0f;
		}

		if constexpr (ENGINE_DEBUG) {
			if (world.hasComponent<NetworkStatsComponent>(fromEnt)) {
				auto& netStatsComp = world.getComponent<NetworkStatsComponent>(fromEnt);
				netStatsComp.inputBufferSize = static_cast<int32>(buffSize);
				netStatsComp.idealInputBufferSize = ideal;
			}
		}
	}

	void ActionSystem::recvActionsServer(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto recvTick = world.getTick();
		auto& actComp = world.getComponent<ActionComponent>(fromEnt);

		const auto minTick = recvTick + 1;
		const auto maxTick = recvTick + actComp.states.capacity() - 1 - 1; // Keep last input so we can duplicate if we need to

		for (auto t = tick - (actComp.states.capacity() / 4); t <= tick; ++t) {
			ActionState s = {};
			
			for (auto& b : s.buttons) {
				// TODO: better interface for reading bits.
				b.pressCount = static_cast<decltype(b.pressCount)>(from.read<2>());
				b.releaseCount = static_cast<decltype(b.releaseCount)>(from.read<2>());
				b.latest = static_cast<decltype(b.latest)>(from.read<1>());
			}
			
			for (auto& a : s.axes) { // TODO: pack
				const auto f = from.read<32>();
				a = reinterpret_cast<const float32&>(f);
			}

			if (t < minTick || t > maxTick) { continue; }
			if (!actComp.states.contains(t)) {
				if (t != tick) {
					ENGINE_INFO("Oh boy. Back filled tick: ", t, " ", tick);
				} else {
					//ENGINE_WARN("Oh boy.", t);
				}

				s.recvTick = recvTick;
				actComp.states.insert(t) = s;
			}
		}
		from.readFlushBits();

		{
			const float32 off = tick < recvTick ? -static_cast<float32>(recvTick - tick) : static_cast<float32>(tick - recvTick);
			// TODO: need to handle negative values. tick-recv vs recv-tick then cast to float and neg
			actComp.estBufferSize += (off - actComp.estBufferSize) * 0.2f;
		}

		//auto* found = actComp.states.find(tick);
		//
		//if (found) {
		//	ENGINE_WARN("Duplicate input for tick ", tick, ". Ignoring.");
		//	return;
		//} else {
		//	found = &actComp.states.insert(tick);
		//}
		//
		//*found = *state;
		//found->recvTick = recvTick;
	}

	void ActionSystem::updateButtonState(Button btn, bool val) {
		for (const auto& ent : actionFilter) {
			updateButtonState(ent, btn, val);
		}
	}

	void ActionSystem::updateButtonState(Engine::ECS::Entity ent, Button btn, bool val) {
		auto& actComp = world.getComponent<ActionComponent>(ent);
		auto& value = actComp.state->buttons[static_cast<int32>(btn)];
		value.pressCount += val;
		value.releaseCount += !val;
		value.latest = val;
		ENGINE_LOG("UPDATE: ", (int32)btn, " ", val);
	}
	
	void ActionSystem::updateAxisState(Axis axis, float32 val) {
		for (const auto& ent : actionFilter) {
			updateAxisState(ent, axis, val);
		}
	}

	void ActionSystem::updateAxisState(Engine::ECS::Entity ent, Axis axis, float32 val) {
		auto& actComp = world.getComponent<ActionComponent>(ent);
		actComp.state->axes[static_cast<int32>(axis)] = val;
	}
}
