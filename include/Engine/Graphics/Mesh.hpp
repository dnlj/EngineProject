#pragma once

// STD
#include <type_traits>
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Graphics {
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

	// TODO: Move
	struct VertexAttribute {
		GLuint location;
		GLuint size;
		GLuint type;
		GLuint offset;
	};

	// TODO: Move
	template<int32 AttributeCount>
	struct VertexFormat {
		GLsizei stride;
		VertexAttribute attributes[AttributeCount];
	};

	// TODO: split
	// TODO: doc
	class Mesh {
		public:
			Mesh();
			Mesh(const Mesh&) = delete;
			Mesh(Mesh&& other) { swap(*this, other); };

			Mesh& operator=(const Mesh&) = delete;
			Mesh& operator=(Mesh&& other) {
				vao = other.vao;
				eboCount = other.eboCount;
				vbo = other.vbo;
				ebo = other.ebo;
				other.vao = 0;
				other.eboCount = 0;
				other.vbo = 0;
				other.ebo = 0;
				return *this;
			}

			~Mesh();

			template<int32 AttributeCount>
			void setBufferFormat(const VertexFormat<AttributeCount>& format);

			void setBufferData(const void* vertexData, GLsizei vertexDataSize, const void* elementData, GLsizei elementDataSize, GLsizei elementCount);

			template<class Vertex, class Element>
			void setBufferData(std::vector<Vertex> vertexData, std::vector<Element> elementData);

			template<class Vertex, class Element, GLsizei VertexCount, GLsizei ElementCount>
			ENGINE_INLINE void setBufferData(const Vertex (&vertexData)[VertexCount], const Element (&elementData)[ElementCount]) {
				setBufferData(&vertexData, VertexCount, &elementData, ElementCount);
			}

			void draw() const;

			ENGINE_INLINE friend void swap(Mesh& a, Mesh& b) noexcept {
				using std::swap;
				swap(a.vao, b.vao);
				swap(a.eboCount, b.eboCount);
				swap(a.vbo, b.vbo);
				swap(a.ebo, b.ebo);
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

#include <Engine/Graphics/Mesh.ipp>
