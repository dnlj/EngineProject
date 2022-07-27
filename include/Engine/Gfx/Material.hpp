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
			MaterialParamsDesc desc;

		public:
			Material(ShaderRef shader) : shader{shader} {}

			const Shader* getShader() const noexcept { return shader.get(); }
			void fetchParameterDesc();
			const auto& getParameterDescription() const noexcept { return desc; }

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
				setStorageSize(base->getParameterDescription().blockSize);
			}

			byte* data() noexcept { // Dont need length because that can be infered from the material
				ENGINE_DEBUG_ASSERT(storage != nullptr, "Attempting to get data of params with empty storage.");
				return reinterpret_cast<byte*>(storage.get());
			}
			const byte* data() const noexcept { return const_cast<MaterialInstance*>(this)->data(); }
			
			void set(uint32 offset, const void* dat, uint32 len) noexcept {
				memcpy(data() + offset, dat, len);
			}

			void set(std::string_view field, const void* dat, uint32 len, NumberType type) noexcept {
				auto& desc = base->getParameterDescription();
				auto found = desc.params.find(field);

				if (found != desc.params.end()) [[likely]] {
					const auto& param = found->second;
					ENGINE_DEBUG_ASSERT(len == param.size, "Wrong material parameter size.");
					ENGINE_DEBUG_ASSERT(type == param.type, "Wrong material parameter type.");
					set(param.offset, dat, len);
				} else [[unlikely]] {
					ENGINE_WARN("Attempting to set invalid material parameter.");
					ENGINE_DEBUG_ASSERT(false);
				}
			}

			void set(nullptr_t field, auto value) {
				static_assert(!sizeof(value), "nullptr is not a valid material parameter.");
			}

			ENGINE_INLINE void set(std::string_view field, float32 value) {
				set(field, &value, sizeof(value), NumberType::Float32);
			};

			ENGINE_INLINE void set(std::string_view field, glm::vec2 value) {
				set(field, &value, sizeof(value), NumberType::Vec2);
			};

			ENGINE_INLINE void set(std::string_view field, glm::vec3 value) {
				set(field, &value, sizeof(value), NumberType::Vec3);
			};

			ENGINE_INLINE void set(std::string_view field, glm::vec4 value) {
				set(field, &value, sizeof(value), NumberType::Vec4);
			};

			ENGINE_INLINE void set(std::string_view field, glm::mat3x3 value) {
				set(field, &value, sizeof(value), NumberType::Mat3x3);
			};

			ENGINE_INLINE void set(std::string_view field, glm::mat4x4 value) {
				set(field, &value, sizeof(value), NumberType::Mat4x4);
			};


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
