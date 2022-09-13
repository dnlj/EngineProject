#pragma once

// Engine
#include <Engine/Gfx/Animation.hpp>
#include <Engine/Gfx/Armature.hpp>
#include <Engine/Gfx/MeshNode.hpp>


namespace Engine::Gfx {
	class ModelData {
		public:
			ModelData() = default;

			ModelData(const ModelData&) = delete;
			ModelData& operator=(const ModelData&) = delete;

			ModelData(ModelData&&) = default;
			ModelData& operator=(ModelData&&) = default;

			std::vector<MeshNode> meshes;
			std::vector<Animation> anims; // TODO: i assume we will also want animation names or something?
			Armature arm;
	};
}
