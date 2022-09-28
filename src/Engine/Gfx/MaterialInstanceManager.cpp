#pragma once

// Engine
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/ResourceContext.hpp>


namespace Engine::Gfx {
	auto MaterialInstanceLoader::load(const Key& key) -> Resource {
		auto sdr = rctx.shaderLoader.get(key.shdr);
		auto mat = rctx.materialLoader.get(sdr);
		auto tex = rctx.textureLoader.get2D("assets/" + key.path); // TODO: better path handling. See: kUYZw2N2

		MaterialInstance to = mat;
		to.set("color", glm::vec4{1,1,0.5,1});
		to.set("tex", tex);

		return to;
	};
}
