// Engine
#include <Engine/Gfx/Material.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	void Material::fetchParameterDesc() {
		const auto sdr = shader->get();

		GLint count = 0;
		glGetProgramInterfaceiv(sdr, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &count);

		constexpr static char section[] = "MaterialParameters";
		std::string name;
		{
			GLint bLen = 0;
			GLint uLen = 0;
			glGetProgramInterfaceiv(sdr, GL_UNIFORM_BLOCK, GL_MAX_NAME_LENGTH, &bLen);
			glGetProgramInterfaceiv(sdr, GL_UNIFORM, GL_MAX_NAME_LENGTH, &uLen);
			name.resize(std::max<GLint>({bLen, uLen, sizeof(section)}));
		}
		const auto nameLen = static_cast<GLsizei>(name.size());

		GLint block = 0;
		for (;block < count; ++block) {
			glGetProgramResourceName(sdr, GL_UNIFORM_BLOCK, block, nameLen, nullptr, name.data());
			//ENGINE_INFO("Block Name: ", name.data());
			if (std::ranges::equal(name, section)) { break; }
		}

		ENGINE_DEBUG_ASSERT(block != count, "Unable to find material parameter block.");
		if (block == count) { return; }

		{
			GLint blockVarCount = 0;
			{
				GLenum props[] = {GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE};
				GLint out[std::size(props)] = {};
				glGetProgramResourceiv(sdr, GL_UNIFORM_BLOCK, block, (GLsizei)std::size(props), props, (GLsizei)std::size(out), nullptr, out);
				blockVarCount = out[0];
				desc.blockSize = out[1];
				//ENGINE_INFO("Block var count: ", blockVarCount, " ", desc.blockSize);
			}

			std::vector<GLint> blockVarIndices(blockVarCount, 0);
			desc.params.reserve(blockVarCount);

			{
				GLenum props[] = {GL_ACTIVE_VARIABLES};
				glGetProgramResourceiv(sdr, GL_UNIFORM_BLOCK, block, (GLsizei)std::size(props), props, blockVarCount, nullptr, blockVarIndices.data());
			}

			for (const auto i : blockVarIndices) {
				GLint len = 0;
				name.clear();
				name.resize(nameLen);
				glGetProgramResourceName(sdr, GL_UNIFORM, i, nameLen, &len, name.data());
				name.resize(len);

				GLenum props[] = {GL_TYPE, GL_OFFSET};
				GLint out[std::size(props)] = {};
				glGetProgramResourceiv(sdr, GL_UNIFORM, i, (GLsizei)std::size(props), props, (GLsizei)std::size(out), nullptr, out);

				auto& param = desc.params[name];
				param.type = fromGLEnum(out[0]);
				param.offset = static_cast<uint32>(std::max(out[1], 0));
				param.size = getTypeSize(param.type);

				//ENGINE_INFO("Block var index: ", name, " ", param.offset, " ", param.size);
			}
		}
	}
}
