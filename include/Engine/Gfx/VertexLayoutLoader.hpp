#pragma once

// Engine
#include <Engine/ResourceManager2.hpp>
#include <Engine/Gfx/NumberType.hpp>
#include <Engine/Gfx/gfxstate.hpp>


namespace Engine::Gfx {
	// TODO: loader instead? should ResourceLoader inherit manager? what if we dont want that? or separate?
	class VertexLayoutLoader final : public ResourceLoader<VertexAttributeDescList, VertexAttributeLayout> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override {
			uint32 vao;
			glCreateVertexArrays(1, &vao);

			if constexpr (ENGINE_DEBUG) {
				auto last = VertexInput::None;
				for (const auto& attrib : key) {
					if (attrib.input == VertexInput::None) { break; }
					ENGINE_DEBUG_ASSERT(last < attrib.input);
					last = attrib.input;
				}
			}

			uint32 location = 0;
			constexpr uint32 binding = 0;

			for (const auto& attrib : key) {
				if (attrib.input == VertexInput::None) { break; }

				glEnableVertexArrayAttrib(vao, location);

				if (isInteger(attrib.type)) {
					glVertexArrayAttribIFormat(vao, location, attrib.size, toGLEnum(attrib.type), attrib.offset);
				} else {
					// TODO: glVertexArrayAttribLFormat note the "L" (doubles)
					glVertexArrayAttribFormat(vao, location, attrib.size, toGLEnum(attrib.type), attrib.normalize, attrib.offset);
				}

				glVertexArrayAttribBinding(vao, location, binding);

				location += (attrib.size + 3) / 4;
			}

			return {vao};
		};
	};

	using VertexAttributeLayoutRef = VertexLayoutLoader::ResourceRef;
	class VertexLayoutManager : public ResourceManager2<VertexAttributeLayout> {
		using ResourceManager2::ResourceManager2;
	};
}
