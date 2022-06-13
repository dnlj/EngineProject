#pragma once

// Engine
#include <Engine/Gfx/DrawCommand.hpp>
#include <Engine/Gfx/resources.hpp>



namespace Engine::Gfx {
	// TODO: should this also have the resource managers?
	class Context {
		private:
			std::vector<DrawCommand> cmds;
			DrawCommand active = {};
			BufferRef matParamsBuffer;
			uint32 matParamsBufferSize = 0;

		public:
			// TODO: should a context just own all the managers? would make sense.
			Context(BufferManager& bufferManager);
			void push(const DrawCommand& cmd) { cmds.push_back(cmd); }
			void render();
	};
}
