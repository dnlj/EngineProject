#pragma once

// STD
#include <string_view>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

namespace Engine {
	class Texture {
		public:
			Texture(const std::string& path);
			~Texture();

			GLuint getID() const;

		private:
			GLuint texture;
	};
}