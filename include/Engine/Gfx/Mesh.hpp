#pragma once

// STD
#include <type_traits>
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gfx/NumberType.hpp>


namespace Engine::Gfx {
	// TODO: Move
	struct VertexAttribute {
		GLuint location;
		GLuint size;
		NumberType type;
		GLuint offset;
	};

	// TODO: Move
	template<int32 AttributeCount>
	struct VertexFormat {
		/** The distance between elements within the buffer in bytes. Usually the size of your vertex. */
		GLsizei stride;
		VertexAttribute attributes[AttributeCount];
	};
	
	// TODO: Move
	enum class Primitive : GLenum {
		Triangles = GL_TRIANGLES,
		TriangleStrip = GL_TRIANGLE_STRIP,
		// TODO: rest
	};

	// TODO: merge with NumberType
	// TODO: Move
	enum class IndexType : GLenum {
		UInt8 = GL_UNSIGNED_BYTE,
		UInt16 = GL_UNSIGNED_SHORT,
		UInt32 = GL_UNSIGNED_INT,
	};

	// TODO: doc
	class Mesh_old {
		private:
			constexpr static GLuint bufferBindingIndex = 0;

			GLuint vao = 0;
			GLsizei count = 0;
			IndexType eboType = {};
			Primitive type = {};
			
			union {
				GLuint buffers[2] = {0, 0};
				struct {
					GLuint vbo;
					GLuint ebo;
				};
			};

			constexpr static GLsizei bufferCount = static_cast<GLsizei>(std::extent_v<decltype(buffers)>);

		public:
			Mesh_old() = default;
			Mesh_old(const Mesh_old&) = delete;
			ENGINE_INLINE Mesh_old(Mesh_old&& other) noexcept { swap(*this, other); };

			Mesh_old& operator=(const Mesh_old&) = delete;
			ENGINE_INLINE Mesh_old& operator=(Mesh_old&& other) noexcept {
				swap(*this, other);
				return *this;
			}

			ENGINE_INLINE ~Mesh_old() {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(bufferCount, buffers);
			}

			auto getVAO() const noexcept { return vao; } // TODO: rm - just for rework
			auto getVBO() const noexcept { return vbo; } // TODO: rm - just for rework
			auto getEBO() const noexcept { return ebo; } // TODO: rm - just for rework

			// TODO: I think we could also make stride be infered in setVertedData. (optional param = sizeof(Vertex))
			// TODO: make setFormat just take a stride and attribute list directly, can use arrayview and dont need to mess with format
			template<int32 AttributeCount>
			Mesh_old& setFormat(const VertexFormat<AttributeCount>& format) {
				if (!vao) { glCreateVertexArrays(1, &vao); }
				if (!vbo) { glCreateBuffers(1, &vbo); }

				glVertexArrayVertexBuffer(vao, bufferBindingIndex, vbo, 0, format.stride);

				for (const auto& attrib : format.attributes) {
					glEnableVertexArrayAttrib(vao, attrib.location);
					// TODO: glVertexArrayAttribLFormat and glVertexArrayAttribIFormat
					if (isInteger(attrib.type)) {
						glVertexArrayAttribIFormat(vao, attrib.location, attrib.size, toGLEnum(attrib.type), attrib.offset);
					} else {
						glVertexArrayAttribFormat(vao, attrib.location, attrib.size, toGLEnum(attrib.type), GL_FALSE, attrib.offset);
					}
					glVertexArrayAttribBinding(vao, attrib.location, bufferBindingIndex);
				}
				return *this;
			}

			/**
			 * Sets the vertex buffer data.
			 * @pre The Mesh_old must have already had its format set with setFormat.
			 * @param type The type primitive stored in @p data.
			 * @param data The data to use.
			 * @param size The size of @p data in bytes.
			 */
			Mesh_old& setVertexData(Primitive type, const void* data, int64 size) {
				ENGINE_DEBUG_ASSERT(size >= 0, "Invalid buffer size");
				ENGINE_DEBUG_ASSERT(vbo, "Must be called after setFormat");
				glNamedBufferData(vbo, size, data, GL_STATIC_DRAW);
				this->type = type;
				return *this;
			}
			
			/**
			 * Infers parameters based on @p data.
			 * @see setElementData
			 */
			template<class Vertex> // TODO: use ArrayView instead of vector
			ENGINE_INLINE Mesh_old& setVertexData(Primitive type, const std::vector<Vertex>& data) {
				return setVertexData(type, data.data(), data.size() * sizeof(Vertex));
			}

			/**
			 * Sets the element buffer data.
			 * @pre The Mesh_old must have already had its format set with setFormat.
			 * @param type The type contained in @p data.
			 * @param data Pointer to the data.
			 * @param size The size of @p data in bytes.
			 * @param count The number of elements of type @p type in @p data.
			 */
			Mesh_old& setElementData(IndexType type, const void* data, int64 size, int32 count) {
				ENGINE_DEBUG_ASSERT(size >= 0, "Invalid buffer size");
				if (!ebo) {
					ENGINE_DEBUG_ASSERT(vao != 0, "Must be called after setFormat");
					glCreateBuffers(1, &ebo);
					glVertexArrayElementBuffer(vao, ebo);
				}
				glNamedBufferData(ebo, size, data, GL_STATIC_DRAW);
				this->count = count;
				eboType = type;
				return *this;
			}

			/**
			 * Infers parameters based on @p data.
			 * @see setElementData
			 */
			template<class Index>
			ENGINE_INLINE Mesh_old& setElementData(const std::vector<Index>& data) {
				ENGINE_DEBUG_ASSERT(data.size() < std::numeric_limits<int32>::max(), "To many mesh elements");
				const auto sz = static_cast<int32>(data.size());

				// TODO: should probably check size not same_as here. See F60fEjDA
				if constexpr (std::same_as<Index, uint8>) {
					return setElementData(IndexType::UInt8, data.data(), sz * sizeof(uint8), sz);
				} else if constexpr (std::same_as<Index, uint16>) {
					return setElementData(IndexType::UInt16, data.data(), sz * sizeof(uint16), sz);
				} else if constexpr (std::same_as<Index, uint32>) {
					return setElementData(IndexType::UInt32, data.data(), sz * sizeof(uint32), sz);
				}
			}

			void draw() const {
				ENGINE_DEBUG_ASSERT(vao, "No vertex array");
				ENGINE_DEBUG_ASSERT(vbo, "No buffer data");
				glBindVertexArray(vao);

				if (ebo) {
					glDrawElements(underlying(type), count, underlying(eboType), 0);
				} else {
					glDrawArrays(underlying(type), 0, count);
				}
			}

			friend void swap(Mesh_old& a, Mesh_old& b) noexcept {
				using std::swap;
				swap(a.vao, b.vao);
				swap(a.count, b.count);
				swap(a.eboType, b.eboType);
				swap(a.type, b.type);
				swap(a.vbo, b.vbo);
				swap(a.ebo, b.ebo);
			}
	};
}
