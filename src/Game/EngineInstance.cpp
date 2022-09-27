// STD
#include <algorithm>

// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/Context.hpp>
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/MaterialManager.hpp>
#include <Engine/Gfx/MeshManager.hpp>
#include <Engine/Gfx/ResourceContext.hpp>
#include <Engine/Gfx/ShaderManager.hpp>
#include <Engine/Gfx/TextureManager.hpp>
#include <Engine/Gfx/VertexLayoutManager.hpp>
#include <Engine/Input/BindManager.hpp>
#include <Engine/UI/Context.hpp>

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
			Engine::Gfx::ResourceContext gfxResCtx;
			Engine::Gfx::Context gfxContext = {gfxResCtx.bufferManager, gfxResCtx.textureLoader};
			Engine::UI::Context uiContext = {gfxResCtx.shaderLoader, gfxResCtx.textureLoader, camera};
	};
	
	EngineInstance::EngineInstance() : pimpl{std::make_unique<EngineInstancePimpl>()} {
		// Must be set before constructing our Game::World.
		getUIContext().setUserdata(this);

		// Workaround for the fact that some systems call engine.getWorld in their constructor.
		// Because of this we need its address before it is constructed.
		// Not ideal but it works for the time being.
		// Even after fixing the above: Specifically not initialized in the initialization list because it requires `*this` as a constructor argument.
		{
			// If for some reason World is over-aligned we need to use explicit alignment
			// when calling both new and delete. This makes the code quite a bit less readable and
			// probably won't ever be an issue so a static assert it is.
			static_assert(alignof(World) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__,
				"Wrong alignment for engine world. You will need to use align_val_t with new/delete."
			);
			void* storage = operator new (sizeof(World));
			world = std::unique_ptr<World>(reinterpret_cast<World*>(storage));
			new (storage) World(*this);
		}
	}

	EngineInstance::~EngineInstance() {
	}

	Engine::Input::BindManager& EngineInstance::getBindManager() noexcept { return pimpl->bindManager; }

	Engine::Gfx::VertexLayoutLoader& EngineInstance::getVertexLayoutLoader() noexcept { return pimpl->gfxResCtx.vertexLayoutLoader; }
	Engine::Gfx::BufferManager& EngineInstance::getBufferManager() noexcept { return pimpl->gfxResCtx.bufferManager; }
	
	Engine::Gfx::ShaderLoader& EngineInstance::getShaderLoader() noexcept { return pimpl->gfxResCtx.shaderLoader; }

	Engine::Gfx::TextureLoader& EngineInstance::getTextureLoader() noexcept { return pimpl->gfxResCtx.textureLoader; }

	Engine::Gfx::MeshManager& EngineInstance::getMeshManager() noexcept { return pimpl->gfxResCtx.meshManager; }
	Engine::Gfx::MaterialLoader& EngineInstance::getMaterialLoader() noexcept { return pimpl->gfxResCtx.materialLoader; }
	Engine::Gfx::MaterialInstanceManager& EngineInstance::getMaterialInstanceManager() noexcept { return pimpl->gfxResCtx.materialInstanceManager; }

	Engine::Gfx::ModelLoader& EngineInstance::getModelLoader() noexcept { return pimpl->gfxResCtx.modelLoader; }

	Engine::Gfx::ResourceContext& EngineInstance::getGraphicsResourceContext() noexcept { return pimpl->gfxResCtx; }
	Engine::Gfx::Context& EngineInstance::getGraphicsContext() noexcept { return pimpl->gfxContext; }
	Engine::UI::Context& EngineInstance::getUIContext() noexcept { return pimpl->uiContext; }

	Engine::Camera& EngineInstance::getCamera() noexcept { return pimpl->camera; }

	// TODO: probably replace with a getTextureMap() that returns a ref to lookup instead of wrapping every get call
	uint32 EngineInstance::getTextureId(std::string_view tex) const { return EngineInstancePimpl::texLookup.get(tex); }
	const char* EngineInstance::getTexturePath(uint32 tex) const { return EngineInstancePimpl::texLookup.get(tex); };
}
