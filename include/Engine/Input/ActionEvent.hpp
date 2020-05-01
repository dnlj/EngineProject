#pragma once

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/Input/ActionId.hpp>
#include <Engine/Input/Value.hpp>


namespace Engine::Input {
	class ActionEvent {
		public:
			Engine::ECS::Entity ent;
			ActionId aid;
			Value val;
	};
}
