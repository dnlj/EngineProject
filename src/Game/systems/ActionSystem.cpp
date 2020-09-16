// Game
#include <Game/systems/ActionSystem.hpp>
#include <Game/World.hpp>
#include <Game/comps/ActionComponent.hpp>


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
				int8 trend = std::max(-128, std::min(static_cast<int32>(actComp.tickTrend), 127));
				conn.write(trend);
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
						netStatsComp.trend = actComp.tickTrend;
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
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto recvTick = *from.read<Engine::ECS::Tick>();
		const float32 trend = *from.read<int8>();
		const auto buffSize = tick - recvTick;

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
		constexpr float32 trendAdjust = 1.1f;
		constexpr float32 maxTrendScale = 8.0f;
		constexpr float32 maxBufferSize = static_cast<float32>(decltype(ActionComponent::states)::capacity());

		// If we sending to soon or to late (according to the server)
		// Then slow down/speed up
		// Otherwise adjust based on or estimated ideal lead time

		// TODO: not really happy with this. we really should use some kind of smoothed adjustment since
		// TODO: cont. we rely on server supplied metrics. Or is this not a problem? needs more testing.
		if (trend < 0) {
			world.tickScale = 1.0f / (trendAdjust - trend * ((maxTrendScale - 1.0f) / 128));
		} else if (trend > 0) {
			world.tickScale = trendAdjust + trend * ((maxTrendScale - 1.0f) / 127);
		} else if (recvTick == 0) {
			ENGINE_WARN("MISSED INPUT");
			// TODO: need to rollback and simulate loss on client side
		} else if (buffSize != ideal) {
			world.tickScale = std::max(0.1f, 1.0f + (buffSize - ideal) * (1.0f / maxBufferSize));
		} else {
			world.tickScale = 1.0f;
		}

		if constexpr (ENGINE_DEBUG) {
			if (world.hasComponent<NetworkStatsComponent>(fromEnt)) {
				auto& netStatsComp = world.getComponent<NetworkStatsComponent>(fromEnt);
				netStatsComp.trend = trend;
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

		const float32 off = tick < minTick ? -1.0f * (minTick-tick) : (tick > maxTick ? tick - maxTick : 0.0f);
		actComp.tickTrend += (off - actComp.tickTrend) * actComp.tickTrendSmoothing;

		if (off) {
			//ENGINE_WARN("Out of window input received. ", tick < minTick, " ", tick > maxTick, " (", minTick, ", ", tick, ", ", maxTick, ")");
			return;
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
