#pragma once


// Forward declarations
namespace Engine {
	class Camera;

	namespace Input {
		class BindManager;
	}

	namespace Gfx {
		class VertexLayoutManager;
		class VertexLayoutLoader;
		class BufferManager;
		class ShaderManager;
		class ShaderLoader;
		class TextureManager;
		class TextureLoader;
		class MeshManager;
		class Context;
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

			Engine::Gfx::VertexLayoutManager& getVertexLayoutManager();
			Engine::Gfx::VertexLayoutLoader& getVertexLayoutLoader();
			Engine::Gfx::BufferManager& getBufferManager();

			Engine::Gfx::ShaderManager& getShaderManager();
			Engine::Gfx::ShaderLoader& getShaderLoader();

			Engine::Gfx::TextureManager& getTextureManager();
			Engine::Gfx::TextureLoader& getTextureLoader();

			Engine::Gfx::MeshManager& getMeshManager();

			Engine::Gfx::Context& getGraphicsContext();

			Engine::Camera& getCamera();

			// TODO: what was the point of making lookup constexpr if this call isnt also constexpr...
			uint32 getTextureId(std::string_view tex) const;
			const char* getTexturePath(uint32 tex) const;
	};
}
