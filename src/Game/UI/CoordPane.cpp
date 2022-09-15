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
		
		addLabel("Camera: {:.3f}");
		addLabel("Cursor: {:.3f}");
		addLabel("Cursor (offset): {:.3f}");
		addLabel("Cursor (world): {:.3f}");

		addLabel("Map Offset: {}");
		addLabel("Map Offset (block): {}");
		addLabel("Map Offset (chunk): {}");

		addLabel("Mouse (offset): {:.3f}");
		addLabel("Mouse (world): {:.3f}");
		addLabel("Mouse (block): {}");
		addLabel("Mouse (block-world): {:.3f}");
		addLabel("Mouse (chunk): {} {}");
		addLabel("Mouse (region): {}");

		ctx->addPanelUpdateFunc(this, [](Panel* panel){
			auto pane = reinterpret_cast<CoordPane*>(panel);
			auto& ctx = *panel->getContext();
			auto& engine = *ctx.getUserdata<EngineInstance>();
			auto& world = engine.getWorld();

			auto& mapSys = world.getSystem<Game::MapSystem>();
			auto& cam = engine.getCamera();

			const auto camPos = engine.getCamera().getPosition();
			const auto cursorPos = ctx.getCursor();
			const auto cursorWorldOffset = (cursorPos - glm::vec2{cam.getScreenSize()/2}) * pixelRescaleFactor;
			const auto cursorWorldPos = glm::vec2{camPos} + cursorWorldOffset;
			const auto mapOffset = world.getSystem<Game::PhysicsOriginShiftSystem>().getOffset();
			const auto mapBlockOffset = mapSys.getBlockOffset();
			const auto mapChunkOffset = mapSys.blockToChunk(mapBlockOffset);

			pane->setLabel(CoordPane::Camera, camPos);
			pane->setLabel(CoordPane::CursorPos, cursorPos);
			pane->setLabel(CoordPane::CursorWorldOffset, cursorWorldOffset);
			pane->setLabel(CoordPane::CursorWorldPos, cursorWorldPos);
			pane->setLabel(CoordPane::MapOffset, mapOffset);
			pane->setLabel(CoordPane::MapOffsetBlock, mapBlockOffset);
			pane->setLabel(CoordPane::MapOffsetChunk, mapChunkOffset);

			const auto& activePlayerFilter = world.getFilter<PlayerFlag>();
			if (activePlayerFilter.empty()) { return; }
			const auto ply = *activePlayerFilter.begin();

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

			pane->setLabel(CoordPane::MouseOffset, offsetMousePos);
			pane->setLabel(CoordPane::MouseWorld, worldMousePos);
			pane->setLabel(CoordPane::MouseBlock, blockMousePos);
			pane->setLabel(CoordPane::MouseBlockWorld, blockWorldMousePos);
			pane->setLabel(CoordPane::MouseChunk, chunkMousePos, chunkBlockMousePos);
			pane->setLabel(CoordPane::MouseRegion, regionMousePos);
		});
	}
}
