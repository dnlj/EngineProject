// Engine
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>
#include <Game/UI/CoordPane.hpp>


namespace Game::UI {
	CoordPane::CoordPane(EUI::Context* context) : AutoList{context} {
		setTitle("Coordinates");
		
		addLabel("Camera: {:.3f}");
		addLabel("Cursor: {:.3f}");
		addLabel("Cursor (offset): {:.3f}");
		addLabel("Cursor (world): {:.3f}");

		addLabel("Zone Offset ({}): {}");

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

			const auto& cam = engine.getCamera();
			const auto camPos = cam.getPosition();
			const auto cursorPos = ctx.getCursor();
			const auto cursorWorldOffset = (cursorPos - glm::vec2{cam.getScreenSize()/2}) * pixelRescaleFactor;
			const auto cursorWorldPos = glm::vec2{camPos} + cursorWorldOffset;

			pane->setLabel(CoordPane::Camera, camPos);
			pane->setLabel(CoordPane::CursorPos, cursorPos);
			pane->setLabel(CoordPane::CursorWorldOffset, cursorWorldOffset);
			pane->setLabel(CoordPane::CursorWorldPos, cursorWorldPos);

			const auto& playerFilter = world.getFilter<PlayerFlag, PhysicsBodyComponent>();
			if (playerFilter.empty()) { return; }
			const auto ply = *playerFilter.begin();
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);

			const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
			const auto zoneId = physComp.getZone();
			const auto& zone = zoneSys.getZone(zoneId);
			pane->setLabel(CoordPane::ZoneOffset, zoneId, zone.offset);

			const auto& actComp = world.getComponent<Game::ActionComponent>(ply);
			if (!actComp.valid()) { return; }

			const auto& mapSys = world.getSystem<Game::MapSystem>();
			const auto offsetMousePos = actComp.getTarget();
			const auto worldMousePos = offsetMousePos + Engine::Glue::as<glm::vec2>(physComp.getPosition());
			const auto blockMousePos = worldToBlock(worldMousePos, zone.offset);
			const auto blockWorldMousePos = blockToWorld(blockMousePos, zone.offset);
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
