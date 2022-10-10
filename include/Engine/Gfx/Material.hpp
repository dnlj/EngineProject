#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Shader.hpp>
#include <Engine/Gfx/NumberType.hpp>
#include <Engine/Gfx/TextureManager.hpp>


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
			Material(ShaderRef shader) : shader{shader} { fetchParameterDesc(); }

			const Shader* getShader() const noexcept { return shader.get(); }
			const auto& getParameterDescription() const noexcept { return desc; }

		private:
			void fetchParameterDesc();
	};

	/**
	 * A specific instance of a material.
	 * Represents a base material + specific parameters (textures, uniforms, etc).
	 * TODO (pb6mVS2e): Should we have a version that bakes static parameters into base shader?
	 */
	class MaterialInstance {
		private:
			using Unit = uint32;
			MaterialRef base;
			std::unique_ptr<Unit[]> storage;
			FlatHashMap<uint32, TextureRef> textures; // offset -> texture

		public:
			MaterialInstance(const MaterialRef& mat) : base{mat} {
				setStorageSize(base->getParameterDescription().blockSize);
			}

			MaterialRefWeak getBase() const noexcept {
				return base;
			}

			byte* data() noexcept { // Dont need length because that can be infered from the material
				ENGINE_DEBUG_ASSERT(storage != nullptr, "Attempting to get data of params with empty storage.");
				return reinterpret_cast<byte*>(storage.get());
			}
			const byte* data() const noexcept { return const_cast<MaterialInstance*>(this)->data(); }
			
			const MaterialParamDesc* getParamDesc(std::string_view field) const noexcept {
				const auto& desc = base->getParameterDescription();
				const auto found = desc.params.find(field);
				if (found == desc.params.end()) [[unlikely]] { return nullptr; }
				return &found->second;
			}

			void set(uint32 offset, const void* dat, uint32 len) noexcept {
				memcpy(data() + offset, dat, len);
			}

			void set(std::string_view field, const void* dat, uint32 len, NumberType type) noexcept {
				const auto* param = getParamDesc(field);
				if (param != nullptr) [[likely]] {
					ENGINE_DEBUG_ASSERT(len == param->size, "Wrong material parameter size.");
					ENGINE_DEBUG_ASSERT(type == param->type, "Wrong material parameter type.");
					set(param->offset, dat, len);
				} else [[unlikely]] {
					ENGINE_WARN("Attempting to set invalid material parameter (", field, ").");
					ENGINE_DEBUG_ASSERT(false);
				}
			}

			void set(std::same_as<nullptr_t> auto field, auto value) {
				// TODO: C++23: string_view	can no longer take a nullptr_t (=delete) so we should be able to remove this overload?
				static_assert(!sizeof(value), "nullptr is not a valid material parameter.");
			}

			ENGINE_INLINE void set(uint32 offset, const auto& value) {
				static_assert(requires { set("", value); }, "Invalid value type for MaterialInstance::set(offset, value);");
				set(offset, &value, sizeof(value));
			}

			ENGINE_INLINE void set(uint32 offset, const TextureRef& value) {
				textures[offset] = value;
			}

			template<class T>
			ENGINE_INLINE void set(std::string_view field, const T& value)
				requires requires { TypeToEnum<T>::value; } {
				set(field, &value, sizeof(value), TypeToEnum_v<T>);
			}

			ENGINE_INLINE void set(std::string_view field, const TextureRef& value) {
				const auto* param = getParamDesc(field);
				if (param) [[likely]] {
					ENGINE_DEBUG_ASSERT(4 == param->size, "Wrong material parameter size.");
					ENGINE_DEBUG_ASSERT(NumberType::Int32 == param->type, "Wrong material parameter type.");
					set(param->offset, value);
				} else [[unlikely]] {
					ENGINE_WARN("Attempting to set invalid material parameter texture (", field, ").");
					ENGINE_DEBUG_ASSERT(false);
				}
			};

			const auto& getTextures() const noexcept {
				return textures;
			}

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
