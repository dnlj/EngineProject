#pragma once

// STD
#include <type_traits>
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Engine.hpp>


// TODO: should this be in a graphics ns?
namespace Engine {
	// TODO: will need a vertex specification type class

	//class Buffer {
	//	public:
	//		Buffer() = delete;
	//		Buffer(const Buffer&) = delete;
	//		Buffer& operator=(const Buffer&) = delete;
	//
	//	private:
	//		GLuint buffer = 0;
	//};

	// TODO: Some kind of shaderinput struct? to set uniforms, textures, etc? look into uniform buffers

	// class Shader;
	// class Texture;
	// class Material;
	// class Model;

	struct VertexAttribute {
		GLuint location;
		GLuint size;
		GLuint type;
		GLuint offset;
	};

	template<int32 AttributeCount>
	struct VertexFormat {
		GLsizei stride;
		VertexAttribute attributes[AttributeCount];
	};

	// TODO: split
	// TODO: doc
	class Mesh {
		public:
			Mesh() {
				glCreateVertexArrays(1, &vao);
				glCreateBuffers(bufferCount, buffers);
				glVertexArrayElementBuffer(vao, ebo);
			}
			
			template<int32 AttributeCount>
			void setBufferFormat(const VertexFormat<AttributeCount>& format) {
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
			void setBufferData(std::vector<Vertex> vertexData, std::vector<Element> elementData) {
				setBufferData(
					vertexData.data(),
					static_cast<GLsizei>(sizeof(Vertex) * vertexData.size()),
					elementData.data(),
					static_cast<GLsizei>(sizeof(Element) * elementData.size()),
					static_cast<GLsizei>(elementData.size())
				);
			}

			void setBufferData(const void* vertexData, GLsizei vertexDataSize, const void* elementData, GLsizei elementDataSize, GLsizei elementCount) {
				eboCount = elementCount;
				glNamedBufferData(vbo, vertexDataSize, vertexData, GL_STATIC_DRAW);
				glNamedBufferData(ebo, elementDataSize, elementData, GL_STATIC_DRAW);
			}

			~Mesh() {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(bufferCount, buffers);
			}

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			// TODO: move

			void draw() const {
				glBindVertexArray(vao);
				// TODO: need to make sure correct ebo element type
				glDrawElements(GL_TRIANGLES, eboCount, GL_UNSIGNED_SHORT, 0);
			}

		private:
			GLuint vao = 0;
			GLsizei eboCount = 0;
			
			union {
				GLuint buffers[2] = {0, 0};
				struct {
					GLuint vbo;
					GLuint ebo;
				};
			};

			constexpr static GLsizei bufferCount = static_cast<GLsizei>(std::extent_v<decltype(buffers)>);
	};
}
