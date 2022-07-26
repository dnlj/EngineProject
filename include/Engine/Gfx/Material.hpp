#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Shader.hpp>
#include <Engine/Gfx/NumberType.hpp>


namespace Engine::Gfx {
	class MaterialParamDesc {
		public:
			NumberType type;
			uint32 offset;
			uint32 size;
	};

	class MaterialParamsDesc {
		public:
			uint32 blockSize;
			FlatHashMap<std::string, MaterialParamDesc> params;
	};

	class Material {
		private:
			ShaderRef shader;
			MaterialParamsDesc params;

		public:
			Material(ShaderRef shader) : shader{shader} {}

			const Shader* getShader() const noexcept { return shader.get(); }

			/**
			 * @return The size in bytes of the parameter block.
			 */
			uint32 getParametersBlockSize() const noexcept { return 16; } // TODO: impl

			void fetchParameterDesc();

		private:
	};

	class MaterialInstance {
		public:
			MaterialRef base; // TODO: why is this public?

		private:
			using Unit = uint32;
			std::unique_ptr<Unit[]> storage;

		public:
			MaterialInstance(const MaterialRef& mat) : base{mat} {
				setStorageSize(base->getParametersBlockSize());
			}

			byte* data() noexcept { // Dont need length because that can be infered from the material
				ENGINE_DEBUG_ASSERT(storage != nullptr, "Attempting to get data of params with empty storage.");
				return reinterpret_cast<byte*>(storage.get());
			}
			const byte* data() const noexcept { return const_cast<MaterialInstance*>(this)->data(); }
			
			void set(uint32 offset, const void* dat, uint32 len) noexcept {
				memcpy(data() + offset, dat, len);
			}

			void set(std::string_view field, const void* dat, uint32 len) noexcept {
				//set(param.offset, dat, len);
			}

			ENGINE_INLINE void set(uint32 field, float32 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(uint32 field, glm::vec2 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(uint32 field, glm::vec3 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(uint32 field, glm::vec4 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(uint32 field, glm::mat3x3 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(uint32 field, glm::mat4x4 value) { set(field, &value, sizeof(value)); };

			// TODO: how to handle this? should this hold a texture ref? take a texture handle? how to handle layout.
			//ENGINE_INLINE void set(MaterialInput field, TextureRef value) { set(field, &value, sizeof(value)); };

		private:
			/**
			 * Sets the size of the storage in bytes.
			 */
			void setStorageSize(size_t bytes) {
				auto sz = (bytes / sizeof(Unit)) + (bytes % sizeof(Unit) != 0);
				storage = decltype(storage)(new Unit[sz]);
			}
	};
}
