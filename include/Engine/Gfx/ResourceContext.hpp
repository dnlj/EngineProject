#pragma once

// Engine
#include <Engine/Gfx/AnimationManager.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/MaterialLoader.hpp>
#include <Engine/Gfx/MeshManager.hpp>
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/TextureLoader.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>

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
			MaterialLoader materialLoader = materialManager;
			MaterialInstanceManager materialInstanceManager;
			ModelLoader modelLoader;
			AnimationManager animManager;

			ResourceContext() : modelLoader{*this} {
			}

			void clean() {
				// TODO: how to distribute cleaning so that it isnt one huge gc stall every x frames (keywords: incremental gc)
				
				// Order is important here because some managers depend on others

				//ModelLoader;
				//
				//AnimationManager;
				// 
				//MeshManager;
				//BufferManager;
				//
				//MaterialInstanceManager;
				//MaterialLoader;
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
