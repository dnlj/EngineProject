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
				for (auto t = currTick - (actComp.states.capacity() / 2); t <= currTick; ++t) {
					const auto& s = actComp.states.get(t);
					//conn.write(actComp.states.get(t));

					for (auto b : s.buttons) {
						conn.write<2>(b.pressCount);
						conn.write<2>(b.releaseCount);
						conn.write<1>(b.latest);
					}

					//conn.writeFlushBits();

					for (auto a : s.axes) { // TODO: compress
						conn.write<32>(reinterpret_cast<uint32&>(a));
					}
				}
				conn.writeFlushBits();

				//conn.write(*state);

				constexpr auto a = sizeof(ActionState);
				conn.msgEnd<MessageType::ACTION>();
			} else if constexpr (ENGINE_SERVER) {
				conn.msgBegin<MessageType::ACTION>();
				conn.write(currTick);
				conn.write(state ? state->recvTick : 0);
				conn.write(actComp.tickTrend); // TODO: int8 ify
				conn.msgEnd<MessageType::ACTION>();

				if (!state) {
					ENGINE_LOG("Missing input for tick ", currTick);
					// TODO: duplicate and decay last input?
					actComp.state = &(actComp.states.insert(currTick) = {});
				}

				actComp.states.remove(currTick - 1);

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
		const auto trend = *from.read<float32>();
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

		const auto ideal = idealTickLead();
		//ENGINE_LOG("idealTickLead = ", ideal, " | ", buffSize, " | ", trend);

		// If we sending to soon or to late (according to the server)
		// Then slow down/speed up
		// Otherwise adjust based on or estimated ideal lead time
		constexpr float32 tol = 1.0f;
		constexpr float32 faster = 0.6f;
		constexpr float32 slower = 1.4f;
		// TODO: scale speed with trend-tol & buffSize - ideal
		// TODO: if we are doing trend binary like this (just > or <) we could just send inc or dec instead of the float
		if (trend < -tol) {
			world.tickScale = faster;
		} else if (trend > tol) {
			world.tickScale = slower;
		} else if (recvTick == 0) {
			ENGINE_WARN("MISSED INPUT");
			// TODO: need to rollback and simulate loss on client side
		} else if (buffSize < ideal) { 
			world.tickScale = faster;
		} else if (buffSize > ideal) { 
			world.tickScale = slower;
		} else {
			world.tickScale = 1.0f;
		}

		// TODO: enable - ENGINE_LOG("Feedback: ", tick, " ", recvTick, " ", world.tickScale, " ", buffSize);
	}

	void ActionSystem::recvActionsServer(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto recvTick = world.getTick();
		auto& actComp = world.getComponent<ActionComponent>(fromEnt);

		const auto minTick = recvTick + 1;
		const auto maxTick = recvTick + actComp.states.capacity() - 1 - 1; // Keep last input so we can duplicate if we need to

		for (auto t = tick - (actComp.states.capacity() / 2); t <= tick; ++t) {
			ActionState s = {};

			for (auto& b : s.buttons) {
				// TODO: better interface for reading bits.
				b.pressCount = static_cast<decltype(b.pressCount)>(from.read<2>());
				b.releaseCount = static_cast<decltype(b.releaseCount)>(from.read<2>());
				b.latest = static_cast<decltype(b.latest)>(from.read<1>());
			}

			//from.readFlushBits();

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
