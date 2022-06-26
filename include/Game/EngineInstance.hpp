#pragma once


// Forward declarations
namespace Engine {
	class Camera;

	namespace Input {
		class BindManager;
	}

	namespace Gui {
		class Context;
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
		class MaterialManager;
		class MaterialInstanceManager;
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
			std::unique_ptr<class World> world;

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

			Engine::Gfx::MaterialManager& getMaterialManager();
			Engine::Gfx::MaterialInstanceManager& getMaterialInstanceManager();

			Engine::Gfx::Context& getGraphicsContext();
			Engine::Gui::Context& getUIContext();

			Engine::Camera& getCamera();
			World& getWorld();

			// TODO: what was the point of making lookup constexpr if this call isnt also constexpr...
			uint32 getTextureId(std::string_view tex) const;
			const char* getTexturePath(uint32 tex) const;
	};
}
