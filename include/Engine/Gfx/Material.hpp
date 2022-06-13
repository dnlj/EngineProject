#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	enum class MaterialInput : uint32 {

	};

	class MaterialParams {
		private:
			using Unit = uint32;
			std::unique_ptr<Unit[]> storage;

		public:
			// TODO: rm - this should be set based on the material it is to be used with - probably in constructor.
			void _TODO_rm_resize(size_t units) { storage = decltype(storage)(new Unit[units]); }

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
	};

	class Material {
		private:
			ShaderRef shader;
			FlatHashMap<MaterialInput, uint32> inputs;

		public:
			Material(ShaderRef shader) : shader{shader} {}

			ShaderHandle getShader() const noexcept { return *shader; }

			uint32 getParametersBlockSize() { return 16; } // TODO: impl

			//void fetchInputs();
			//void apply(const MaterialParams& params);

		private:
			uint32 getFieldOffset(MaterialInput input) const noexcept {
				auto found = inputs.find(input);
				if (found == inputs.end()) [[unlikely]] {
					ENGINE_WARN("Attempting to get invalid material input field");
					return 0;
				}
				return found->second;
			}
	};

	using MaterialRef = void*; // TODO: rm

	class MaterialInstance {
		public:
			MaterialRef material;
			MaterialParams params;
	};
	using MaterialInstanceRef = void*; // TODO: rm
}
