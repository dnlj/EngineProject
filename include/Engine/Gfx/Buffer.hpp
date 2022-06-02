#pragma once


namespace Engine::Gfx {
	struct StorageFlag_ {
		enum StorageFlag : GLbitfield {
			None = 0,
			MapRead = GL_MAP_READ_BIT,
			MapWrite = GL_MAP_WRITE_BIT,
			MapPersistent = GL_MAP_PERSISTENT_BIT,
			MapCoherent = GL_MAP_COHERENT_BIT,
			DynamicStorage = GL_DYNAMIC_STORAGE_BIT,
			ClientStorage = GL_CLIENT_STORAGE_BIT,
		};
	};
	using StorageFlag = StorageFlag_::StorageFlag;

	class Buffer {
		private:
			GLuint buff = 0;

		public:
			Buffer() = default;
			Buffer(Buffer&) = delete;
			Buffer(Buffer&& other) {
				buff = other.buff;
				other.buff = 0;
			};

			~Buffer() { glDeleteBuffers(1, &buff); }

			ENGINE_INLINE auto get() const { return buff; }

			void alloc(uint64 size, const void* data, StorageFlag flags = {}) {
				// TODO: is it better to use glBufferData instead of delete/create in cases where we need a resizeable buffer?
				if (buff) { glDeleteBuffers(1, &buff); }
				glCreateBuffers(1, &buff);
				glNamedBufferStorage(buff, size, data, flags);
			}

			ENGINE_INLINE void alloc(uint64 size, StorageFlag flags = {}) {
				alloc(size, nullptr, flags);
			}

			ENGINE_INLINE void alloc(std::ranges::contiguous_range auto range, StorageFlag flags = {}) {
				alloc(std::ranges::size(range) * sizeof(*std::ranges::cdata(range)), std::ranges::cdata(range), flags);
			}

			ENGINE_INLINE void setData(uint64 offset, uint64 size, const void* data) {
				ENGINE_DEBUG_ASSERT(buff != 0, "Attempting to set data of empty buffer.");
				ENGINE_DEBUG_ASSERT(size > 0, "Attempting to set data with size zero.");
				glNamedBufferSubData(buff, offset, size, data);
			}

			ENGINE_INLINE void setData(uint64 size, const void* data) {
				setData(0, size, data);
			}

			ENGINE_INLINE void setData(std::ranges::contiguous_range auto range) {
				setData(0, std::ranges::size(range) * sizeof(*std::ranges::cdata(range)), std::ranges::cdata(range));
			}
	};
}
