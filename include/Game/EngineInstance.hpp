#pragma once


// Forward declarations
namespace Engine {
	class TextureManager;
	class ShaderManager;
	class Camera;

	namespace Input {
		class BindManager;
	}

	namespace Gfx {
		class VertexLayoutManager;
		class VertexLayoutLoader;
		class BufferManager;
	}
}

namespace Game {
	enum class InputLayer {
		GuiFocus,
		GuiHover,
		Game,
		_count,
	};

	class EngineInstance {
		private:
			std::unique_ptr<class EngineInstancePimpl> pimpl;

		public:
			EngineInstance();
			~EngineInstance();

			Engine::Input::BindManager& getBindManager();
			Engine::TextureManager& getTextureManager();
			Engine::ShaderManager& getShaderManager();
			Engine::Gfx::VertexLayoutManager& getVertexLayoutManager();
			Engine::Gfx::VertexLayoutLoader& getVertexLayoutLoader();
			Engine::Gfx::BufferManager& getBufferManager();
			Engine::Camera& getCamera();
	};
}
