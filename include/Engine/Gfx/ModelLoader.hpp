#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/ModelReader.hpp>
#include <Engine/Gfx/ModelData.hpp>


namespace Engine::Gfx {
	class ModelLoader final {
		private:
			FlatHashMap<std::string, ModelData> cache;
			ModelReader reader;
			ResourceContext& rctx;
			Engine::Gfx::MaterialInstanceRef mats[3];// TODO: rm - load from model

		public:
			ModelLoader(ResourceContext& rctx) : rctx{rctx} {
			}

			const ModelData& get(const std::string_view path) {
				auto found = cache.find(path);
				if (found == cache.end()) {
					// TODO: C++26: Would like to use try_emplace to make this whole function
					// ^^^^: simpler, but we cannot because it does not support heterogeneous lookup.
					// ^^^^: Should be fixed by https://wg21.link/p2363
					found = cache.emplace(std::piecewise_construct, std::forward_as_tuple(path), std::tuple{}).first;
					load(path, found->second);
				}
				return found->second;
			}

			void clean() {
				// TODO: since nothing stores direct references how do we want to handle this?
				// ^^^^: would be useful to have a function that checks all sub refs - but these might
				// ^^^^: have diff number of ref count due to multiple deps
			}

		private:
			void load(const std::string_view path, ModelData& data);
	};
}
