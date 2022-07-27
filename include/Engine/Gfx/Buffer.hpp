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
			
			Buffer(const void* data, uint64 size, StorageFlag flags = {}) {
				alloc(data, size, flags);
			}

			Buffer(uint64 size, StorageFlag flags = {}) {
				alloc(nullptr, size, flags);
			}

			Buffer(std::ranges::contiguous_range auto range, StorageFlag flags = {}) {
				alloc(std::ranges::cdata(range), std::ranges::size(range) * sizeof(*std::ranges::cdata(range)), flags);
			}

			~Buffer() { glDeleteBuffers(1, &buff); }

			ENGINE_INLINE auto get() const { return buff; }

			void alloc(const void* data, uint64 size, StorageFlag flags = {}) {
				// TODO: is it better to use glBufferData instead of delete/create in cases where we need a resizeable buffer?
				if (buff) { glDeleteBuffers(1, &buff); }
				glCreateBuffers(1, &buff);
				glNamedBufferStorage(buff, size, data, flags);
			}

			ENGINE_INLINE void alloc(uint64 size, StorageFlag flags = {}) {
				alloc(nullptr, size, flags);
			}

			ENGINE_INLINE void alloc(std::ranges::contiguous_range auto range, StorageFlag flags = {}) {
				alloc(std::ranges::cdata(range), std::ranges::size(range) * sizeof(*std::ranges::cdata(range)), flags);
			}

			ENGINE_INLINE void setData(uint64 offset, const void* data, uint64 size) {
				ENGINE_DEBUG_ASSERT(buff != 0, "Attempting to set data of empty buffer.");
				ENGINE_DEBUG_ASSERT(size > 0, "Attempting to set data with size zero.");
				glNamedBufferSubData(buff, offset, size, data);
			}

			ENGINE_INLINE void setData(const void* data, uint64 size) {
				setData(0, data, size);
			}

			ENGINE_INLINE void setData(std::ranges::contiguous_range auto range) {
				setData(0, std::ranges::cdata(range), std::ranges::size(range) * sizeof(*std::ranges::cdata(range)));
			}
	};
}
