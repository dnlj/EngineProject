#pragma once

// Engine
#include <Engine/Graphics/Mesh.hpp>


namespace Engine::Graphics {
	template<int32 AttributeCount>
	void Mesh::setBufferFormat(const VertexFormat<AttributeCount>& format) {
		constexpr GLuint bufferBindingIndex = 0;
		glVertexArrayVertexBuffer(vao, bufferBindingIndex, vbo, 0, format.stride);

		for (const auto& attrib : format.attributes) {
			glEnableVertexArrayAttrib(vao, attrib.location);

			ENGINE_DEBUG_ASSERT(attrib.type == GL_FLOAT,
				// TODO: implement for non float types.
				// See: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribFormat.xhtml
				// Will need to check attrib.type and select one of:
				// glVertexArrayAttribFormat, glVertexArrayAttribIFormat, or glVertexArrayAttribLFormat
				"TODO: implement for non GLfloat types. Currently only GL_FLOAT is implemented."
			);

			glVertexArrayAttribFormat(vao, attrib.location, attrib.size, attrib.type, GL_FALSE, attrib.offset);
			glVertexArrayAttribBinding(vao, attrib.location, bufferBindingIndex);
		}
	}

	template<class Vertex, class Element>
	void Mesh::setBufferData(std::vector<Vertex> vertexData, std::vector<Element> elementData) {
		setBufferData(
			vertexData.data(),
			static_cast<GLsizei>(sizeof(Vertex) * vertexData.size()),
			elementData.data(),
			static_cast<GLsizei>(sizeof(Element) * elementData.size()),
			static_cast<GLsizei>(elementData.size())
		);
	}
}
