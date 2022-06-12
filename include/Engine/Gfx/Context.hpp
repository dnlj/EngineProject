#pragma once

// Engine
#include <Engine/Gfx/DrawCommand.hpp>



namespace Engine::Gfx {
	// TODO: should this also have the resource managers?
	class Context {
		private:
			std::vector<DrawCommand> cmds;
			DrawCommand active = {};

		public:
			void push(const DrawCommand& cmd) { cmds.push_back(cmd); }
			void render();
	};
}
