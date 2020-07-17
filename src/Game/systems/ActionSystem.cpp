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

	
	void ActionSystem::tick(float32 dt) {
		// TODO: On server - nothing - we get input updates from network system.
		// TODO: On client - we get input updates from input system
		// TODO: On client - If server didnt get correct input we need to rollback and mirror that loss on our side or we will desync

		const auto currTick = world.getTick();
		const auto minTick = currTick - 64;
		for (const auto ent : actionFilter) {
			auto& actComp = world.getComponent<ActionComponent>(ent);
			auto& actQueueComp = world.getComponent<ActionQueueComponent>(ent);


			if constexpr (ENGINE_SERVER) { // ========================================================================
				auto& state = actQueueComp.states[world.getTick() % 64];
				auto& writer = world.getComponent<ConnectionComponent>(ent).conn->writer;
				writer.next(MessageType::ACTION, Engine::Net::Channel::UNRELIABLE);
				writer.write(world.getTick());
				writer.write(state.recvTick);

				if (state.recvTick == 0) {
					ENGINE_LOG("Missing input for tick ", world.getTick());
				}

				// TODO: need to deal with rollback
				state.recvTick = 0;
			}
		}

		if constexpr (ENGINE_CLIENT) {
			sendActions();
		}
	}

	void ActionSystem::sendActions() {
		for (const auto& ent : actionFilter) {
			auto& writer = world.getComponent<ConnectionComponent>(ent).conn->writer;
			writer.next(MessageType::ACTION, Engine::Net::Channel::UNRELIABLE);
			writer.write(world.getTick());
		}
	}

	void ActionSystem::recvActions(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		if constexpr (ENGINE_SERVER) {
			recvActionsServer(from, head, fromEnt);
		} else {
			recvActionsClient(from, head, fromEnt);
		}
	}

	void ActionSystem::recvActionsClient(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		auto& reader = from.reader;
		const auto tick = *reader.read<Engine::ECS::Tick>();
		const auto recvTick = *reader.read<Engine::ECS::Tick>();
		const auto buffSize = tick - recvTick;

		// TODO: if to far behind snap to correct tick
		// TODO: if to far ahead scale more aggressively. Probably based on number of ticks/sec
		// TODO: send tick for last input we got so we can determine how far off we are
		if (recvTick == 0) {
			world.tickScale = 0.8f;
		} else if (buffSize > 5) {
			world.tickScale = 1.1f;
		} else {
			world.tickScale = 1.0f;
		}

		ENGINE_LOG("Feedback: ", tick, " ", recvTick, " ", world.tickScale, " ", buffSize);
	}

	void ActionSystem::recvActionsServer(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt) {
		auto& reader = from.reader;
		const auto tick = *reader.read<Engine::ECS::Tick>();
		const auto recvTick = world.getTick();
		const auto minTick = recvTick + 1;
		const auto maxTick = minTick + 64 - 1;

		if (tick < minTick || tick > maxTick) { return; }

		auto& aqc = world.getComponent<ActionQueueComponent>(fromEnt);
		// TODO: if we dont get a message we will have old data in the buffer. fixxxxx
		aqc.states[tick % 64].recvTick = recvTick;
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