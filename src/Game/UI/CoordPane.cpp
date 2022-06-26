// Engine
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/UI/CoordPane.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>


namespace Game::UI {
	CoordPane::CoordPane(EUI::Context* context) : AutoList{context} {
		setTitle("Coordinates");

		addLabel("Mouse (offset): {:.3f}");
		addLabel("Mouse (world): {:.3f}");
		addLabel("Mouse (block): {}");
		addLabel("Mouse (block-world): {:.3f}");
		addLabel("Mouse (chunk): {} {}");
		addLabel("Mouse (region): {}");
		addLabel("Camera: {:.3f}");
		addLabel("Map Offset: {}");
		addLabel("Map Offset (block): {}");
		addLabel("Map Offset (chunk): {}");

		ctx->addPanelUpdateFunc(this, [](Panel* panel){
			auto pane = reinterpret_cast<CoordPane*>(panel);
			auto& engine = *panel->getContext()->getUserdata<EngineInstance>();
			auto& world = engine.getWorld();

			const auto& activePlayerFilter = world.getFilter<PlayerFlag>();
			if (activePlayerFilter.empty()) { return; }
			const auto ply = *activePlayerFilter.begin();

			auto& mapSys = world.getSystem<Game::MapSystem>();

			const auto& actComp = world.getComponent<Game::ActionComponent>(ply);
			if (!actComp.valid()) { return; }

			// TODO: reimplement - ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
			const auto offsetMousePos = actComp.getTarget();
			const auto worldMousePos = offsetMousePos + Engine::Glue::as<glm::vec2>(physComp.getPosition());
			const auto blockMousePos = mapSys.worldToBlock(worldMousePos);
			const auto blockWorldMousePos = mapSys.blockToWorld(blockMousePos);
			const auto chunkMousePos = mapSys.blockToChunk(blockMousePos);
			const auto chunkBlockMousePos = mapSys.chunkToBlock(chunkMousePos);
			const auto regionMousePos = mapSys.chunkToRegion(chunkMousePos);
			const auto camPos = engine.getCamera().getPosition();
			const auto mapOffset = world.getSystem<Game::PhysicsOriginShiftSystem>().getOffset();
			const auto mapBlockOffset = mapSys.getBlockOffset();
			const auto mapChunkOffset = mapSys.blockToChunk(mapBlockOffset);

			pane->setLabel(CoordPane::MouseOffset, offsetMousePos);
			pane->setLabel(CoordPane::MouseWorld, worldMousePos);
			pane->setLabel(CoordPane::MouseBlock, blockMousePos);
			pane->setLabel(CoordPane::MouseBlockWorld, blockWorldMousePos);
			pane->setLabel(CoordPane::MouseChunk, chunkMousePos, chunkBlockMousePos);
			pane->setLabel(CoordPane::MouseRegion, regionMousePos);
			pane->setLabel(CoordPane::Camera, camPos);
			pane->setLabel(CoordPane::MapOffset, mapOffset);
			pane->setLabel(CoordPane::MapOffsetBlock, mapBlockOffset);
			pane->setLabel(CoordPane::MapOffsetChunk, mapChunkOffset);
		});
	}
}
