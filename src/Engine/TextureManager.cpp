// STD
#include <algorithm>

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Gfx/Image.hpp>

// SOIL
#include <soil/SOIL.h>

namespace Engine {
	auto TextureManager::load(const std::string& path) -> StorePtr {
		Image img = path;
		img.flipY(); // TODO: idealy just fix in model uv coords

		auto res = std::make_unique<TextureInfo>();
		res->size = img.size();
		res->tex.setStorage(TextureFormat::SRGBA8, img.size());
		res->tex.setImage(img);
		res->tex.setFilter(TextureFilter::NEAREST);
		res->tex.setWrap(TextureWrap::REPEAT);

		return res;
	}
}
