#pragma once

// Engine
#include <Engine/Graphics/Mesh.hpp>


namespace Engine::Graphics {
	template<int32 AttributeCount>
	void Mesh::setBufferFormat(const VertexFormat<AttributeCount>& format) {
		// TODO: atm this loop doesnt make sense since we have hard coded buffer(vbo) and binding index
		static_assert(AttributeCount == 1);
		for (const auto& attrib : format.attributes) {
			constexpr GLuint bufferBindingIndex = 0;
			glVertexArrayVertexBuffer(vao, bufferBindingIndex, vbo, 0, format.stride);
			glEnableVertexArrayAttrib(vao, attrib.location);
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