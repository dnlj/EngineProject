#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Material.hpp>

namespace Engine::Gfx {
	class MatInstDesc { // TODO: ultimately this should be some kind of path/uri
		public:
			std::string path;
			std::string shdr;

			bool operator==(const MatInstDesc&) const noexcept = default;
	};
}

namespace Engine {
	template<>
	struct Hash<Gfx::MatInstDesc> {
		[[nodiscard]] uint64 operator()(const Gfx::MatInstDesc& val) const noexcept {
			auto h = hash(val.path);
			hashCombine(h, hash(val.shdr));
			return h;
		}
	};
}

namespace Engine::Gfx {
	class MaterialInstanceManager : public ResourceManager<MaterialInstance> {
		using ResourceManager::ResourceManager;
	};

	class MaterialInstanceLoader final : public ResourceLoader<MatInstDesc, MaterialInstance> {
		private:
			ResourceContext& rctx;

		public:
			MaterialInstanceLoader(ResourceContext& rctx) : rctx{rctx} {}

		private:
			virtual Resource load(const Key& key) override;
	};
}
