#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/StaticVector.hpp>
#include <Engine/Gfx/ModelData.hpp>


namespace Game {
	class BufferBinding {
		public:
			Engine::Gfx::BufferRef buff;
			uint16 index;
			uint16 offset;
			uint16 size;
	};

	class MeshInstData {
		public:
			using BufferBindings = Engine::StaticVector<BufferBinding, 4>;

		public:
			glm::mat4 mvp; // TODO: is this the best place for this? really shouldnt store the whole mvp here. should really pull from scene cam + position comppnent
			BufferBindings bindings;
			Engine::Gfx::MeshRef mesh;
			Engine::Gfx::MaterialInstanceRef mat;
			Engine::Gfx::NodeId nodeId;

	};

	class ModelComponent {
		public:
			ModelComponent() = default;

			ModelComponent(const Engine::Gfx::ModelData& mdat) {
				meshes.reserve(mdat.meshes.size());
				for (const auto& inst : mdat.meshes) {
					meshes.push_back({
						.mesh = inst.mesh,
						.mat = inst.mat,
						.nodeId = inst.nodeId,
					});
				}
			};

			std::vector<MeshInstData> meshes;

			// TODO: maybe this should be its own component? idk. not sure where this makes sense logically.
			// BufferBindings bindings;

	};
}
