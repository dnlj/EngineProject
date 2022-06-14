#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	enum class MaterialInput : uint32 {

	};

	class Material {
		private:
			ShaderRef shader;
			//FlatHashMap<MaterialInput, uint32> inputs;

		public:
			Material(ShaderRef shader) : shader{shader} {}

			const Shader* getShader() const noexcept { return shader.get(); }
			uint32 getParametersBlockSize() const noexcept { return 16; } // TODO: impl

			//void fetchInputs();
			//void apply(const MaterialParams& params);

		private:
			//uint32 getFieldOffset(MaterialInput input) const noexcept {
			//	auto found = inputs.find(input);
			//	if (found == inputs.end()) [[unlikely]] {
			//		ENGINE_WARN("Attempting to get invalid material input field");
			//		return 0;
			//	}
			//	return found->second;
			//}
	};

	class MaterialParams {
		private:
			using Unit = uint32;
			std::unique_ptr<Unit[]> storage;

		public:
			MaterialParams(const Material& mat) { setStorageSize(mat.getParametersBlockSize()); }
			MaterialParams(const MaterialRef& mat) : MaterialParams{*mat} {}

			byte* data() noexcept { // Dont need length because that can be infered from the material
				ENGINE_DEBUG_ASSERT(storage != nullptr, "Attempting to get data of params with empty storage.");
				return reinterpret_cast<byte*>(storage.get());
			}
			const byte* data() const noexcept { return const_cast<MaterialParams*>(this)->data(); }

			using MaterialInput = uint32; // TODO: rm - temp while testing
			uint32 getFieldOffset(MaterialInput field) {
				// TODO: watch alignment - this depends on the shader - std140 vs std430
				return 0; // TODO: impl
			}

			void set(MaterialInput field, const void* dat, uint32 len) noexcept {
				memcpy(data() + getFieldOffset(field), dat, len);
			}

			ENGINE_INLINE void set(MaterialInput field, float32 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(MaterialInput field, glm::vec2 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(MaterialInput field, glm::vec3 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(MaterialInput field, glm::vec4 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(MaterialInput field, glm::mat3x3 value) { set(field, &value, sizeof(value)); };
			ENGINE_INLINE void set(MaterialInput field, glm::mat4x4 value) { set(field, &value, sizeof(value)); };

			// TODO: how to handle this? should this hold a texture ref? take a texture handle? how to handle layout.
			//ENGINE_INLINE void set(MaterialInput field, TextureRef value) { set(field, &value, sizeof(value)); };

		private:
			void setStorageSize(size_t units) { storage = decltype(storage)(new Unit[units]); }
	};

	class MaterialInstance {
		public:
			MaterialInstance(const MaterialRef& mat) : base{mat}, params{mat} {}
			MaterialRef base;
			MaterialParams params;
	};
}
