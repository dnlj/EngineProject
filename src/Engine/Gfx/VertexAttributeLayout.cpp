#pragma once

// Engine
#include <Engine/Gfx/VertexAttributeLayout.hpp>


namespace Engine::Gfx {
	VertexAttributeLayout::VertexAttributeLayout(const VertexAttributeLayoutDesc& layout) {
		glCreateVertexArrays(1, &vao);

		for (const auto& bd : layout.divisors) {
			glVertexArrayBindingDivisor(vao, bd.binding, bd.divisor);
		}

		for (const auto& attrib : layout.attribs) {
			if (attrib.size == 0) { break; }

			glEnableVertexArrayAttrib(vao, attrib.input);

			if (attrib.target == VertexAttribTarget::Int) {
				glVertexArrayAttribIFormat(vao, attrib.input, attrib.size, toGLEnum(attrib.type), attrib.offset);
			} else {
				ENGINE_DEBUG_ASSERT(attrib.target == VertexAttribTarget::Float, "Invalid vertex attribute target.");
				// TODO: glVertexArrayAttribLFormat note the "L" (doubles)
				glVertexArrayAttribFormat(vao, attrib.input, attrib.size, toGLEnum(attrib.type), attrib.normalize, attrib.offset);
			}

			glVertexArrayAttribBinding(vao, attrib.input, attrib.binding);

			//location += (attrib.size + 3) / 4;
		}
	}
}
