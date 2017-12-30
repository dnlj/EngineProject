#pragma once

// STD
#include <unordered_map>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// TODO: Document
namespace Engine {
	class TextureManager {
		public:
			TextureManager();

			~TextureManager();

			// TODO: return a texture resource wrapper thing. Reference counting?
			GLuint getTexture(const std::string& path);

		private:
			std::unordered_map<std::string, GLuint> textures;
	};
}