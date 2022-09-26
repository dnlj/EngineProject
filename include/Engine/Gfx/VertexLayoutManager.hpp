#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/NumberType.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>


namespace Engine::Gfx {
	class VertexLayoutManager : public ResourceManager<VertexAttributeLayout> {
		using ResourceManager::ResourceManager;
	};

	class VertexLayoutLoader final : public ResourceLoader<VertexAttributeDescList, VertexAttributeLayout> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override {
			uint32 vao;
			glCreateVertexArrays(1, &vao);

			for (const auto& attrib : key) {
				if (attrib.size == 0) { break; }

				glEnableVertexArrayAttrib(vao, attrib.input);

				if (attrib.target == VertexAttribTarget::Int) {
					glVertexArrayAttribIFormat(vao, attrib.input, attrib.size, toGLEnum(attrib.type), attrib.offset);
				} else {
					ENGINE_DEBUG_ASSERT(attrib.target == VertexAttribTarget::Float, "Invalid vertex attribute target.");
					// TODO: glVertexArrayAttribLFormat note the "L" (doubles)
					glVertexArrayAttribFormat(vao, attrib.input, attrib.size, toGLEnum(attrib.type), attrib.normalize, attrib.offset);
				}

				glVertexArrayBindingDivisor(vao, attrib.binding, attrib.divisor);
				glVertexArrayAttribBinding(vao, attrib.input, attrib.binding);

				//location += (attrib.size + 3) / 4;
			}

			return VertexAttributeLayout{VertexAttributeLayout::AllowConstruct, vao};
		};
	};
}
