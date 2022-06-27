#pragma once


// Forward declarations
namespace Engine {
	class Camera;

	namespace Input {
		class BindManager;
	}

	namespace UI {
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
			EngineInstance(const EngineInstance&) = delete;
			~EngineInstance();

			Engine::Input::BindManager& getBindManager() noexcept;

			Engine::Gfx::VertexLayoutManager& getVertexLayoutManager() noexcept;
			Engine::Gfx::VertexLayoutLoader& getVertexLayoutLoader() noexcept;
			Engine::Gfx::BufferManager& getBufferManager() noexcept;

			Engine::Gfx::ShaderManager& getShaderManager() noexcept;
			Engine::Gfx::ShaderLoader& getShaderLoader() noexcept;

			Engine::Gfx::TextureManager& getTextureManager() noexcept;
			Engine::Gfx::TextureLoader& getTextureLoader() noexcept;

			Engine::Gfx::MeshManager& getMeshManager() noexcept;

			Engine::Gfx::MaterialManager& getMaterialManager() noexcept;
			Engine::Gfx::MaterialInstanceManager& getMaterialInstanceManager() noexcept;

			Engine::Gfx::Context& getGraphicsContext() noexcept;
			Engine::UI::Context& getUIContext() noexcept;

			Engine::Camera& getCamera() noexcept;
			World& getWorld() noexcept { return *world; }

			// TODO: what was the point of making lookup constexpr if this call isnt also constexpr...
			uint32 getTextureId(std::string_view tex) const;
			const char* getTexturePath(uint32 tex) const;
	};
}
