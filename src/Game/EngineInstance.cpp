// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/Input/BindManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/TextureManager.hpp>


namespace Game {
	class EngineInstancePimpl {
		public:
			Engine::Input::BindManager bindManager;
			Engine::TextureManager textureManager;
			Engine::ShaderManager shaderManager;

			Engine::Gfx::VertexLayoutManager vertexLayoutManager;
			Engine::Gfx::VertexLayoutLoader vertexLayoutLoader = vertexLayoutManager;

			Engine::Gfx::BufferManager bufferManager;

			Engine::Gfx::ShaderManager2 shaderManager2;
			Engine::Gfx::ShaderLoader shaderLoader = shaderManager2;

			Engine::Camera camera;
	};
	
	EngineInstance::EngineInstance() : pimpl{std::make_unique<EngineInstancePimpl>()} {}
	EngineInstance::~EngineInstance() {}

	Engine::Input::BindManager& EngineInstance::getBindManager() { return pimpl->bindManager; }
	Engine::TextureManager& EngineInstance::getTextureManager() { return pimpl->textureManager; }
	Engine::ShaderManager& EngineInstance::getShaderManager() { return pimpl->shaderManager; }
	Engine::Gfx::VertexLayoutManager& EngineInstance::getVertexLayoutManager() { return pimpl->vertexLayoutManager; }
	Engine::Gfx::VertexLayoutLoader& EngineInstance::getVertexLayoutLoader() { return pimpl->vertexLayoutLoader; }
	Engine::Gfx::BufferManager& EngineInstance::getBufferManager() { return pimpl->bufferManager; }
	
	Engine::Gfx::ShaderManager2& EngineInstance::getShaderManager2() { return pimpl->shaderManager2; }
	Engine::Gfx::ShaderLoader& EngineInstance::getShaderLoader() { return pimpl->shaderLoader; }
	Engine::Camera& EngineInstance::getCamera() { return pimpl->camera; }
}
