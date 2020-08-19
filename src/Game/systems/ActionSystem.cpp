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
				// TODO: send past X states
				conn.write(*state);
				//const auto min = (snapshots + currTick - (snapshots / 2)) % snapshots;
				//for (auto i = currTick; ) {
				//	auto& state = actComp.states[
				//	// TODO: write compressed
				//}

				constexpr auto a = sizeof(ActionState);
				conn.msgEnd<MessageType::ACTION>();
			} else if constexpr (ENGINE_SERVER) {
				conn.msgBegin<MessageType::ACTION>();
				conn.write(currTick);
				conn.write(state ? state->recvTick : 0);
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
		// TODO: we dont actually use tick/recvTick. just nw buffsize
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto recvTick = *from.read<Engine::ECS::Tick>();
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

		ENGINE_LOG("idealTickLead = ", ideal);
		if (recvTick == 0) { // TODO: how to handle 
			ENGINE_WARN("MISSED INPUT");
			return;
		}

		if (buffSize < ideal) {
			world.tickScale = 0.8f;
		} else if (buffSize > ideal) {
			world.tickScale = 1.1f;
		} else {
			world.tickScale = 1.0f;
		}

		// TODO: if to far behind snap to correct tick
		// TODO: if to far ahead scale more aggressively. Probably based on number of ticks/sec
		// TODO: send tick for last input we got so we can determine how far off we are
		//if (buffSize < 1) {
		//	world.tickScale = 0.8f;
		//} else if (buffSize > 5) {
		//	world.tickScale = 1.1f;
		//} else {
		//	world.tickScale = 1.0f;
		//}

		// TODO: enable - ENGINE_LOG("Feedback: ", tick, " ", recvTick, " ", world.tickScale, " ", buffSize);
	}

	void ActionSystem::recvActionsServer(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto state = from.read<ActionState>();
		const auto recvTick = world.getTick();

		const auto minTick = recvTick + 1;
		const auto maxTick = recvTick + decltype(ActionComponent::states)::capacity() - 1 - 1; // Keep last input so we can duplicate if we need to

		// TODO: if tick < minTick tell client to fast
		// TODO: if tick > maxTick tell client to slow
		if (tick < minTick || tick > maxTick) {
			ENGINE_WARN("Out of window input received. ", tick < minTick, " ", tick > maxTick);
			return;
		}

		auto& actComp = world.getComponent<ActionComponent>(fromEnt);
		auto* found = actComp.states.find(tick);

		if (found) {
			ENGINE_WARN("Duplicate input for tick ", tick, ". Ignoring.");
			return;
		} else {
			found = &actComp.states.insert(tick);
		}

		*found = *state;
		found->recvTick = recvTick;
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
