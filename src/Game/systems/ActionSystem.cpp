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
		for (const auto ent : actionFilter) {
			auto& actComp = world.getComponent<ActionComponent>(ent);
			while (!actComp.actionQueue.empty()) {
				auto& curr = actComp.actionQueue.back();
				auto& prev = actComp.get(curr.aid);
				for (auto& l : actionIdToListeners[curr.aid]) {
					l(ent, curr.aid, curr.state, prev.state);
				}
				prev.state = curr.state;
				actComp.actionQueue.pop();
			}
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
		auto& actionComp = world.getComponent<ActionComponent>(ent);
		actionComp.actionQueue.push({aid, curr});
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
