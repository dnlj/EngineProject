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

		// TODO: add units px/m/b/c/etc
		addLabel("Camera: {:.3f}m");
		addLabel("Cursor: {:.3f}px");
		addLabel("Cursor (offset): {:.3f}m");
		addLabel("Cursor (world): {:.3f}m");
		addLabel("Cursor (world abs): {}m");

		addLabel("Zone Offset ({}): {}m");

		addLabel("Target (offset): {:.3f}");
		addLabel("Target (world): {:.3f}");
		addLabel("Target (block): {}");
		addLabel("Target (block-world): {:.3f}");
		addLabel("Target (chunk): {} {}");
		addLabel("Target (region): {}");

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
			pane->setLabel(CoordPane::CursorPos, cursorPos); // TOOD (oHuNLpn2): why is this inverted? + = - and - = +
			pane->setLabel(CoordPane::CursorWorldOffset, cursorWorldOffset);
			pane->setLabel(CoordPane::CursorWorldPos, cursorWorldPos);

			const auto& playerFilter = world.getFilter<PlayerFlag, PhysicsBodyComponent>();
			if (playerFilter.empty()) { return; }
			const auto ply = *playerFilter.begin();
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);

			const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
			const auto zoneId = physComp.getZoneId();
			const auto& zone = zoneSys.getZone(zoneId);

			pane->setLabel(CoordPane::CursorWorldAbsPos, worldToAbsolute(cursorWorldPos, zone.offset2));
			pane->setLabel(CoordPane::ZoneOffset, zoneId, zone.offset2);

			const auto& actComp = world.getComponent<Game::ActionComponent>(ply);
			if (!actComp.valid()) { return; }

			const auto offsetTargetPos = actComp.getTarget();
			const auto worldTargetPos = offsetTargetPos + Engine::Glue::as<glm::vec2>(physComp.getPosition());
			const auto blockTargetPos = worldToBlock2(worldTargetPos, zone.offset2);
			const auto blockWorldTargetPos = blockToWorld2(blockTargetPos, zone.offset2);
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
