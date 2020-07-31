// Game
#include <Game/systems/ActionSystem.hpp>
#include <Game/World.hpp>
#include <Game/comps/ActionComponent.hpp>


namespace Game {
	ActionSystem::ActionSystem(SystemArg arg)
		: System{arg}
		, actionFilter{world.getFilterFor<ActionComponent>()}
		, connFilter{world.getFilterFor<ConnectionComponent>()} {
	}

	void ActionSystem::preTick() {
		if constexpr (ENGINE_CLIENT) {
			for (const auto ent : actionFilter) {
				auto& actComp = world.getComponent<ActionComponent>(ent);
				for (auto& btnState : actComp.state.buttons) {
					btnState.pressCount = 0;
					btnState.releaseCount = 0;
				}
			}
		}
	}

	void ActionSystem::tick() {
		// TODO: On client - If server didnt get correct input we need to rollback and mirror that loss on our side or we will desync

		const auto currTick = world.getTick();
		for (const auto ent : actionFilter) {
			auto& actComp = world.getComponent<ActionComponent>(ent);
			auto& curr = actComp.state;
			auto& stored = actComp.states[currTick % snapshots];

			if constexpr (ENGINE_CLIENT) {
				if (world.isPerformingRollback()) {
					curr = stored;
				} else {
					stored = curr;
				}
			} else if constexpr (ENGINE_SERVER) {
				curr = stored;
				auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
				conn.msgBegin<MessageType::ACTION>();
				conn.write(currTick);
				conn.write(curr.recvTick);
				conn.msgEnd<MessageType::ACTION>();

				if (curr.recvTick == 0) {
					// TODO: enable - ENGINE_LOG("Missing input for tick ", currTick);
					// TODO: duplicate and decay last input?
				}

				// If we ever add lag compensation we will need to handle server rollback here.
				stored.recvTick = 0;
			}
		}

		if constexpr (ENGINE_CLIENT) {
			sendActions();
		}
	}

	void ActionSystem::sendActions() {
		for (const auto& ent : actionFilter) {
			const auto& actComp = world.getComponent<ActionComponent>(ent);
			auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
			conn.msgBegin<MessageType::ACTION>();
			conn.write(world.getTick());
			conn.write(actComp.state);
			conn.msgEnd<MessageType::ACTION>();

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
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto recvTick = *from.read<Engine::ECS::Tick>();
		const auto buffSize = tick - recvTick;

		// TODO: if to far behind snap to correct tick
		// TODO: if to far ahead scale more aggressively. Probably based on number of ticks/sec
		// TODO: send tick for last input we got so we can determine how far off we are
		if (recvTick == 0 || buffSize < 1) {
			world.tickScale = 0.8f;
		} else if (buffSize > 5) {
			world.tickScale = 1.1f;
		} else {
			world.tickScale = 1.0f;
		}

		// TODO: enable - ENGINE_LOG("Feedback: ", tick, " ", recvTick, " ", world.tickScale, " ", buffSize);
	}

	void ActionSystem::recvActionsServer(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		const auto tick = *from.read<Engine::ECS::Tick>();
		const auto state = from.read<ActionState>();
		const auto recvTick = world.getTick();

		const auto minTick = recvTick + 1;
		const auto maxTick = recvTick + snapshots - 1;

		// TODO: if tick < minTick tell client to fast
		// TODO: if tick > maxTick tell client to slow
		if (tick < minTick || tick > maxTick) {
			// TODO - enable ENGINE_WARN("Out of window input received.");
			return;
		}

		auto& actComp = world.getComponent<ActionComponent>(fromEnt);
		auto& stored = actComp.states[tick % snapshots];

		if (stored.recvTick) {
			ENGINE_WARN("Duplicate input for tick ", tick, ". Ignoring.");
			return;
		}

		stored = *state;
		stored.recvTick = recvTick;
	}

	void ActionSystem::updateButtonState(Button btn, bool val) {
		for (const auto& ent : actionFilter) {
			updateButtonState(ent, btn, val);
		}
	}

	void ActionSystem::updateButtonState(Engine::ECS::Entity ent, Button btn, bool val) {
		auto& actComp = world.getComponent<ActionComponent>(ent);
		auto& value = actComp.state.buttons[static_cast<int32>(btn)];
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
		actComp.state.axes[static_cast<int32>(axis)] = val;
	}
}
