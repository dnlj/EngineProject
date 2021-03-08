// STD
#include <algorithm>

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/Engine.hpp>
#include <Engine/TextureOptions.hpp>
#include <Engine/Graphics/Image.hpp>

// SOIL
#include <soil/SOIL.h>

namespace Engine {
	Texture2D TextureManager::load(const std::string& path) {
		Image img = path;
		img.flipY(); // TODO: idealy just fix in model uv coords

		Texture2D tex;
		tex.setStorage(TextureFormat::SRGBA8, img.size());
		tex.setImage(img);
		tex.setFilter(TextureFilter::NEAREST);
		tex.setWrap(TextureWrap::REPEAT);

		return tex;
	}
}
