#pragma once

// Engine
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/TextureLoader.hpp>
#include <Engine/Gfx/MeshManager.hpp>
#include <Engine/Gfx/MaterialManager.hpp>
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/ModelLoader.hpp>

namespace Engine::Gfx {
	class ResourceContext {
		public:
			VertexLayoutManager vertexLayoutManager;
			VertexLayoutLoader vertexLayoutLoader = vertexLayoutManager;
			BufferManager bufferManager;
			ShaderManager shaderManager;
			ShaderLoader shaderLoader = shaderManager;
			TextureManager textureManager;
			TextureLoader textureLoader = textureManager;
			MeshManager meshManager;
			MaterialManager materialManager;
			MaterialInstanceManager materialInstanceManager;
			ModelLoader modelLoader;

			ResourceContext() : modelLoader{*this} {
			}

			void clean() {
				// TODO: how to distribute cleaning so that it isnt one huge gc stall every x frames (keywords: incremental gc)
				
				// Order is important here because some managers depend on others

				//ModelLoader;
				//
				//MeshManager;
				//BufferManager;
				//
				//MaterialInstanceManager;
				//MaterialManager;
				//ShaderLoader;
				//ShaderManager;
				//TextureLoader;
				//TextureManager;
				//
				//VertexLayoutLoader;
				//VertexLayoutManager;
			}
	};
}
