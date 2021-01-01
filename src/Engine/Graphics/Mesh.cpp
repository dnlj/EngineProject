// Engine
#include <Engine/Graphics/Mesh.hpp>


namespace Engine::Graphics {
	Mesh::Mesh() {
		// TODO: dont create these here. Maybe in setBufferData or setBufferFormat
		glCreateVertexArrays(1, &vao);
		glCreateBuffers(bufferCount, buffers);
		glVertexArrayElementBuffer(vao, ebo);
	}

	Mesh::~Mesh() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(bufferCount, buffers);
	}

	void Mesh::setBufferData(const void* vertexData, GLsizei vertexDataSize, const void* elementData, GLsizei elementDataSize, GLsizei elementCount) {
		eboCount = elementCount;
		glNamedBufferData(vbo, vertexDataSize, vertexData, GL_STATIC_DRAW);
		glNamedBufferData(ebo, elementDataSize, elementData, GL_STATIC_DRAW);
	}

	void Mesh::draw() const {
		glBindVertexArray(vao);
		// TODO: need to make sure correct ebo element type
		glDrawElements(GL_TRIANGLES, eboCount, GL_UNSIGNED_SHORT, 0);
	}
}
