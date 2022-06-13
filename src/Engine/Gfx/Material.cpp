// Engine
#include <Engine/Gfx/Material.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	/*void Material::fetchInputs() {
		const auto p = shader->get();

		int32 count = 0;
		glGetProgramiv(p, GL_ACTIVE_UNIFORMS, &count);
		if (!count) { return; }

		int32 maxName = 0;
		glGetProgramiv(p, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxName);

		int32 len = 0;
		int32 size = 0;
		uint32 type = 0;
		std::string name(maxName,'\0');

		puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		for (int32 i = 0; i < count; ++i) {
			glGetActiveUniform(p, i, maxName, &len, &size, &type, name.data());
			ENGINE_LOG("Uniform: ", name.data(), " ", size, " ", type);
			std::ranges::fill(name, '\0');
		}
		puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	}*/
}
