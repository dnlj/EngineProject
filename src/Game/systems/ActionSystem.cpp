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
			auto& queue = actComp.actionQueue;
			const auto histCurr = currTick % std::extent_v<decltype(actComp.actionHistory)>;
			const auto histNext = (currTick + 1) % std::extent_v<decltype(actComp.actionHistory)>;
			actComp.actions = actComp.actionHistory[histCurr];

			// TODO: store a flag and only sort if new inputs
			std::sort(queue.begin(), queue.end(), [](const auto& a, const auto& b){
				return a.tick < b.tick;
			});

			while (!queue.empty() && queue.back().tick < minTick) {
				queue.pop();
			}

			// TODO: could do a binary search to jump to currTick
			for (const auto& curr : queue) {
				const auto diff = curr.tick - currTick;
				if (diff < 0) { continue; }
				if (diff > 0) { break; }

				auto& prev = actComp.get(curr.aid);
				for (auto& l : actionIdToListeners[curr.aid]) {
					l(ent, curr.aid, curr.state, prev.state);
				}
				prev.state = curr.state;
			}

			actComp.actionHistory[histNext] = actComp.actions;
		}
	}

	void ActionSystem::queueAction(Engine::Input::ActionId aid, Engine::Input::Value curr) {
		if constexpr (ENGINE_SERVER) {
			ENGINE_ASSERT("Should not be called on server. This is a bug.");
			return;
		}

		for (const auto& ent : actionFilter) {
			auto& actionComp = world.getComponent<ActionComponent>(ent);
			queueAction(ent, aid, curr);
		}

		// TODO: only send actions at end of frame. Pack into one message.
		for (const auto ent : connFilter) {
			// TODO: we probably dont want to be sending every axis input to the server. just once every tick or something.
			auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;
			conn.writer.next(Game::MessageType::ACTION, Engine::Net::Channel::UNRELIABLE);
			conn.writer.write(aid);
			conn.writer.write(curr);
		}
	}

	void ActionSystem::queueAction(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr) {
		ENGINE_DEBUG_ASSERT(aid < count(), "Attempting to process invalid action");
		auto& queue = world.getComponent<ActionComponent>(ent).actionQueue;
		queue.push({aid, curr, world.getTick()});
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
