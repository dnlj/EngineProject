// STD
#include <type_traits>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/TextureOptions.hpp>

namespace Engine {
	TextureOptions::TextureOptions(TextureWrap wrap, TextureFilter filter, bool mipmap)
		: wrap{wrap}
		, filter{filter}
		, mipmap{mipmap} {
	}

	void TextureOptions::setWrap(TextureWrap wrap) {
		this->wrap = wrap;
	}

	void TextureOptions::setFilter(TextureFilter filter) {
		this->filter = filter;
	}

	void TextureOptions::setMipmap(bool mipmap) {
		this->mipmap = mipmap;
	}

	GLint TextureOptions::getGLWrap() const {
		return static_cast<GLint>(wrap);
	};

	GLint TextureOptions::getGLFilterMin() const {
		switch (filter) {
			case TextureFilter::TRILINEAR:
				return GL_LINEAR_MIPMAP_LINEAR;
			case TextureFilter::BILINEAR:
				return GL_LINEAR;
			case TextureFilter::NEAREST:
				return GL_NEAREST;
			default:
				ENGINE_WARN(
					"Unknown Engine::TextureFilter: "
					<< static_cast<std::underlying_type_t<decltype(filter)>>(filter)
				);

				return  GL_NEAREST;
		}
	};

	GLint TextureOptions::getGLFilterMag() const {
		switch (filter) {
			case TextureFilter::TRILINEAR:
			case TextureFilter::BILINEAR:
				return GL_LINEAR;
			case TextureFilter::NEAREST:
				return GL_NEAREST;
			default:
				ENGINE_WARN(
					"Unknown Engine::TextureFilter: "
					<< static_cast<std::underlying_type_t<decltype(filter)>>(filter)
				);

				return  GL_NEAREST;
		}
	};

	bool TextureOptions::getMipmap() const {
		return mipmap;
	}
}
