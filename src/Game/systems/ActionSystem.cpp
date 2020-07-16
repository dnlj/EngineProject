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
		const auto currTick = world.getTick();
		const auto minTick = currTick - 64;
		for (const auto ent : actionFilter) {
			auto& actComp = world.getComponent<ActionComponent>(ent);
			auto& actQueueComp = world.getComponent<ActionQueueComponent>(ent);
			//auto& queue = actQueueComp.actionQueue;
			//const auto offset = actQueueComp.offset + actQueueComp.bufferSize + ENGINE_SERVER;

			/*
			// TODO: store a flag and only sort if new inputs
			std::sort(queue.begin(), queue.end(), [](const auto& a, const auto& b){
				return a.tick < b.tick;
			});

			// TODO: need server tick offset + buffer
			while (!queue.empty() && queue.back().tick + offset < minTick) {
				queue.pop();
			}

			// TODO: SERVER - if no input for expected tick increase aqc.bufferSize. Will need to track next expected client tick - and - deal with rollback
			// TODO: if we are shrinking bufferSize try to play multiple ticks per server tick? Not sure how else to deal with this.

			// TODO: could do a binary search to jump to currTick
			for (const auto& curr : queue) {
				//const auto diff = curr.tick - currTick;
				const auto diff = curr.tick + offset - currTick;

				if (diff < 0) { continue; }
				if (diff > 0) { break; }

				if (curr.aid > 1) {
					ENGINE_LOG("Act: ", (int)curr.aid, " ", curr.state.value, " ", curr.tick);
				}

				auto& prev = actComp.get(curr.aid);
				for (auto& l : actionIdToListeners[curr.aid]) {
					l(ent, curr.aid, curr.state, prev.state);
				}
				prev.state = curr.state;
			}*/

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

	void ActionSystem::queueAction(Engine::Input::ActionId aid, Engine::Input::Value curr) {
		if constexpr (ENGINE_SERVER) {
			ENGINE_ASSERT("Should not be called on server. This is a bug.");
			return;
		}

		for (const auto& ent : actionFilter) {
			auto& actionComp = world.getComponent<ActionComponent>(ent);
			queueAction(ent, aid, curr, world.getTick());
		}
	}

	void ActionSystem::queueAction(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr, Engine::ECS::Tick tick) {
		ENGINE_DEBUG_ASSERT(aid < count(), "Attempting to process invalid action");
		auto& queue = world.getComponent<ActionQueueComponent>(ent).actionQueue;
		queue.emplace(aid, curr, tick);
	}

	Engine::Input::ActionId ActionSystem::create(const std::string& name) {
		auto found = actionNameToId.find(name);
		if (found == actionNameToId.end()) {
			const auto next = actionNameToId.size();
			found = actionNameToId.emplace(name, static_cast<Engine::Input::ActionId>(next)).first;
			actionIdToListeners.emplace_back();

			for (const auto ent : actionFilter) {
				auto& actC = world.getComponent<ActionComponent>(ent);
				actC.grow(static_cast<Engine::Input::ActionId>(next + 1));
			}
		} else {
			ENGINE_WARN("Attempting to create action that already exists: ", name);
		}

		return found->second;
	}

	Engine::Input::ActionId ActionSystem::getId(const std::string& name) {
		auto found = actionNameToId.find(name);
		if (found == actionNameToId.end()) {
			ENGINE_WARN("Attempting to get the id of a nonexistent action: ", name);
			return create(name);
		}

		return found->second;
	}

	
	Engine::Input::ActionId ActionSystem::count() const {
		return static_cast<Engine::Input::ActionId>(actionNameToId.size());
	}
}
