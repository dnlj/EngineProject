// STD
#include <algorithm>

// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/Context.hpp>
#include <Engine/Gfx/MaterialManager.hpp>
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/MeshManager.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/TextureLoader.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/Input/BindManager.hpp>
#include <Engine/Gui/Context.hpp>

// Game
#include <Game/World.hpp>


namespace {
	using namespace Engine::Types;
	template<uint32 N>
	class Lookup {
		private:
			std::array<std::string_view, N> lookup;

		public:
			consteval Lookup(const std::array<std::string_view, N>& array) : lookup{array} {
				std::ranges::sort(lookup);
			}

			constexpr const char* get(uint32 value) const {
				return lookup[value].data();
			}

			// TODO: consteval - that was the whole point of this
			constexpr uint32 get(std::string_view value) const {
				auto found = std::ranges::lower_bound(lookup, value);
				if (found == lookup.end()) {
					ENGINE_WARN("Attempting to lookup invalid value (", value, ")");
					return 0;
				}
				return static_cast<uint32>(found - lookup.begin());
			}
	};

	template<uint32 N>
	Lookup(const char*const(&)[N]) -> Lookup<N>;
}


namespace Game {
	class EngineInstancePimpl {
		public:
			#define TEX(T) T,
			constexpr static auto texLookup = Lookup({
				#include <Game/assets.xpp>
			});

		public:
			Engine::Camera camera;

			Engine::Input::BindManager bindManager;

			Engine::Gfx::VertexLayoutManager vertexLayoutManager;
			Engine::Gfx::VertexLayoutLoader vertexLayoutLoader = vertexLayoutManager;

			Engine::Gfx::BufferManager bufferManager;

			Engine::Gfx::ShaderManager shaderManager;
			Engine::Gfx::ShaderLoader shaderLoader = shaderManager;

			Engine::Gfx::TextureManager textureManager;
			Engine::Gfx::TextureLoader textureLoader = textureManager;

			Engine::Gfx::MeshManager meshManager;

			Engine::Gfx::MaterialManager materialManager;
			Engine::Gfx::MaterialInstanceManager materialInstanceManager;

			Engine::Gfx::Context gfxContext = bufferManager;
			Engine::Gui::Context uiContext = {shaderLoader, textureLoader, camera};
	};
	
	EngineInstance::EngineInstance() : pimpl{std::make_unique<EngineInstancePimpl>()} {
		// Must be set before constructing our Game::World.
		getUIContext().setUserdata(this);

		// Workaround for the fact that some systems call engine.getWorld in their constructor.
		// Because of this we need its address before it is constructed.
		// Not ideal but it works for the time being.
		// Even after fixing the above: Specifically not initialized in the initialization list because it requires `*this` as a constructor argument.
		{
			void* storage = operator new (sizeof(World), std::align_val_t{alignof(World)});
			world = std::unique_ptr<World>(reinterpret_cast<World*>(storage));
			new (storage) World(*this);
		}
	}

	EngineInstance::~EngineInstance() {}

	Engine::Input::BindManager& EngineInstance::getBindManager() noexcept { return pimpl->bindManager; }

	Engine::Gfx::VertexLayoutManager& EngineInstance::getVertexLayoutManager() noexcept { return pimpl->vertexLayoutManager; }
	Engine::Gfx::VertexLayoutLoader& EngineInstance::getVertexLayoutLoader() noexcept { return pimpl->vertexLayoutLoader; }
	Engine::Gfx::BufferManager& EngineInstance::getBufferManager() noexcept { return pimpl->bufferManager; }
	
	Engine::Gfx::ShaderManager& EngineInstance::getShaderManager() noexcept { return pimpl->shaderManager; }
	Engine::Gfx::ShaderLoader& EngineInstance::getShaderLoader() noexcept { return pimpl->shaderLoader; }

	Engine::Gfx::TextureManager& EngineInstance::getTextureManager() noexcept { return pimpl->textureManager; }
	Engine::Gfx::TextureLoader& EngineInstance::getTextureLoader() noexcept { return pimpl->textureLoader; }

	Engine::Gfx::MeshManager& EngineInstance::getMeshManager() noexcept { return pimpl->meshManager; }
	Engine::Gfx::MaterialManager& EngineInstance::getMaterialManager() noexcept { return pimpl->materialManager; }
	Engine::Gfx::MaterialInstanceManager& EngineInstance::getMaterialInstanceManager() noexcept { return pimpl->materialInstanceManager; }


	Engine::Gfx::Context& EngineInstance::getGraphicsContext() noexcept { return pimpl->gfxContext; }
	Engine::Gui::Context& EngineInstance::getUIContext() noexcept { return pimpl->uiContext; }

	Engine::Camera& EngineInstance::getCamera() noexcept { return pimpl->camera; }

	// TODO: probably replace with a getTextureMap() that returns a ref to lookup instead of wrapping every get call
	uint32 EngineInstance::getTextureId(std::string_view tex) const { return EngineInstancePimpl::texLookup.get(tex); }
	const char* EngineInstance::getTexturePath(uint32 tex) const { return EngineInstancePimpl::texLookup.get(tex); };
}
