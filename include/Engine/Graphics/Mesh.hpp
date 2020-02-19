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
			// TODO: Mesh(Mesh&&) = delete;

			~Mesh();

			Mesh& operator=(const Mesh&) = delete;
			// TODO: Mesh& operator=(Mesh&&) = delete;

			template<int32 AttributeCount>
			void setBufferFormat(const VertexFormat<AttributeCount>& format);

			template<class Vertex, class Element>
			void setBufferData(std::vector<Vertex> vertexData, std::vector<Element> elementData);

			void setBufferData(const void* vertexData, GLsizei vertexDataSize, const void* elementData, GLsizei elementDataSize, GLsizei elementCount);

			void draw() const;

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
