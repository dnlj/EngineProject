// Engine
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>
#include <Game/UI/CoordPane.hpp>


namespace Game::UI {
	CoordPane::CoordPane(EUI::Context* context) : AutoList{context} {
		setTitle("Coordinates");

		addLabel("Camera: {:.3f}m");
		addLabel("Cursor (screen): {:.3f}px");
		addLabel("Cursor (offset): {:.3f}m");
		addLabel("Cursor (world): {:.3f}m");
		addLabel("Cursor (world abs): {}m");

		addLabel("Zone Offset ({}): {}m");

		addLabel("Target (offset): {:.3f}m");
		addLabel("Target (world): {:.3f}m");
		addLabel("Target (block): {}b");
		addLabel("Target (block-world): {:.3f}b");
		addLabel("Target (chunk): {}c {}b");
		addLabel("Target (region): {}r");

		ctx->addPanelUpdateFunc(this, [](Panel* panel){
			auto pane = reinterpret_cast<CoordPane*>(panel);
			auto& ctx = *panel->getContext();
			auto& engine = *ctx.getUserdata<EngineInstance>();
			auto& world = engine.getWorld();

			const auto& cam = engine.getCamera();
			const auto camPos = cam.getPosition();
			const auto cursorPos = ctx.getCursor();

			// Need to negate y to go from screen +y=down to world +y=up
			const auto cursorWorldOffset = (cursorPos - glm::vec2{cam.getScreenSize()/2}) * pixelRescaleFactor * glm::vec2{1, -1}; 
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
			const auto zoneId = physComp.getZoneId();
			const auto& zone = zoneSys.getZone(zoneId);

			pane->setLabel(CoordPane::CursorWorldAbsPos, worldToAbsolute(cursorWorldPos, zone.offset));
			pane->setLabel(CoordPane::ZoneOffset, zoneId, zone.offset);

			const auto& actComp = world.getComponent<Game::ActionComponent>(ply);
			if (!actComp.valid()) { return; }

			const auto offsetTargetPos = actComp.getTarget();
			const auto worldTargetPos = offsetTargetPos + Engine::Glue::as<glm::vec2>(physComp.getPosition());
			const auto blockTargetPos = worldToBlock2(worldTargetPos, zone.offset);
			const auto blockWorldTargetPos = blockToWorld2(blockTargetPos, zone.offset);
			const auto chunkTargetPos = blockToChunk(blockTargetPos);
			const auto chunkBlockTargetPos = chunkToBlock(chunkTargetPos);
			const auto regionTargetPos = chunkToRegion(chunkTargetPos);

			pane->setLabel(CoordPane::TargetOffset, offsetTargetPos);
			pane->setLabel(CoordPane::TargetWorld, worldTargetPos);
			pane->setLabel(CoordPane::TargetBlock, blockTargetPos);
			pane->setLabel(CoordPane::TargetBlockWorld, blockWorldTargetPos);
			pane->setLabel(CoordPane::TargetChunk, chunkTargetPos, chunkBlockTargetPos);
			pane->setLabel(CoordPane::TargetRegion, regionTargetPos);
		});
	}
}
