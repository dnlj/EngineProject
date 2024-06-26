// Game
#include <Game/UI/InfoPane.hpp>
#include <Game/EngineInstance.hpp>
#include <Game/systems/NetworkingSystem.hpp>


// TODO: rm
#include <charconv>
#include <type_traits>
#include <Engine/traits.hpp>
#include <Game/comps/all.hpp> // TODO: rm - just for debug command
#include <Game/systems/all.hpp> // TODO: rm - just for debug command


namespace Game::UI {
	InfoPane::InfoPane(EUI::Context* context) : AutoList{context} {
		setTitle("Info");
		addLabel("FPS: {:.3f} ({:.6f})");
		addLabel("Tick: {}");
		addLabel("Tick Scale: {:.3f}");
		addLabel("Tick Rate: {}");
		addLabel("Run Delta: {}");
		addLabel("Run Delta Smooth: {}");
		setLabel(UI::InfoPane::TickRate, Game::tickrate);

		disconnect = ctx->constructPanel<EUI::Button>();
		disconnect->autoText("Disconnect");
		disconnect->lockSize();
		getContent()->addChild(disconnect);

		disconnect->setAction([](EUI::Button* btn){
			auto& world = btn->getContext()->getUserdata<EngineInstance>()->getWorld();
			auto& netSys = world.getSystem<NetworkingSystem>();

			for (const auto& conn : netSys.getConnections()) {
				if (conn.second->ent) {
					world.getSystem<NetworkingSystem>().requestDisconnect(conn.first);
				}
			}
		});

		ctx->addPanelUpdateFunc(getContent(), [](auto* panel){
			auto* self = reinterpret_cast<InfoPane*>(panel->getParent());
			self->update();
		});

		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TODO: rm - zone testing debug ui
		//////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Assumes split=2000, join=300, sameZone=500
		const auto debugZoneButton = [this](const auto& name, EUI::Button::Callback func) {
			auto button = ctx->constructPanel<EUI::Button>();
			button->autoText(name);
			button->lockSize();
			getContent()->addChild(button);
			button->setAction(std::move(func));
			return button;
		};

		auto add = [this](float32 x, float32 y){
			auto& engine = *ctx->getUserdata<EngineInstance>();
			auto& world = engine.getWorld();
			auto& physSys = world.getSystem<PhysicsSystem>();
			auto ent = world.createEntity();

			// TODO: The player flag breaks things atm but is needed for the
			//       ZoneManagementSystem to pick up these entities. Currently
			//       it causes other systems to pick up these as players, which
			//       makes sense, but that comes with assumptions that we don't
			//       fulfill here.
			world.addComponent<PlayerFlag>(ent);
			//world.addComponent<ActionComponent>(ent, world.getTick());
			world.addComponent<PhysicsBodyComponent>(ent, physSys.createPhysicsCircle(ent, {x, y}, 0, PhysicsCategory::Player));
			return ent;
		};

		constexpr auto max = 499;
		debugZoneButton("Zone Wide 1x2", [this, add](EUI::Button* btn){
			add(0,0*max);
			add(0,1*max);
		});

		debugZoneButton("Zone Wide 1x3", [this, add](EUI::Button* btn){
			add(0,0*max);
			add(0,1*max);
			add(0,2*max);
		});

		debugZoneButton("Zone Wide 1x4", [this, add](EUI::Button* btn){
			add(0,0*max);
			add(0,1*max);
			add(0,2*max);
			add(0,3*max);
		});

		debugZoneButton("Zone Wide 1x8", [this, add, debugZoneButton](EUI::Button* btn){
			add(0,0*max);
			add(0,1*max);
			add(0,2*max);
			add(0,3*max);
			add(0,4*max);
			add(0,5*max);
			add(0,6*max);
			add(0,7*max);
		});

		debugZoneButton("Zone Wide 2x4", [this, add, debugZoneButton](EUI::Button* btn){
			add(0,0*max);
			add(0,1*max);
			add(0,2*max);
			add(0,3*max);
			
			add(max+10, 0*max);
			add(max+10, 1*max);
			add(max+10, 2*max);
			add(max+10, 3*max);
		});

		debugZoneButton("Zone Wide 2x1", [this, add, debugZoneButton](EUI::Button* btn){
			add(0, 0);
			add(0, 2100);
		});
	}

	void InfoPane::update() {
		const auto& world = getContext()->getUserdata<EngineInstance>()->getWorld();
		const Sample curr = {world.getTime(), world.getUpdate()};
		fpsSamples.push(curr);

		// Cull old data
		while(!fpsSamples.empty() && fpsSamples.front().time < (curr.time - std::chrono::milliseconds{1000})) {
			fpsSamples.pop();
		}

		ENGINE_DEBUG_ASSERT(fpsSamples.size() > 0);
		if (curr.time - lastUpdate >= std::chrono::milliseconds{100}) {
			lastUpdate = curr.time;

			const auto& front = fpsSamples.front();
			const auto& back = fpsSamples.back();

			auto frames = back.frames - front.frames;
			auto time = back.time - front.time;

			float32 fps = frames / Engine::Clock::Seconds{time}.count();
			setLabel(UI::InfoPane::FPS, fps, 1.0f/fps);
		}

		setLabel(UI::InfoPane::Tick, world.getTick());
		setLabel(UI::InfoPane::TickScale, world.tickScale);
		setLabel(UI::InfoPane::RunDelta, std::chrono::duration<double, std::milli>{world.getDeltaTimeNS()});
		setLabel(UI::InfoPane::RunDeltaSmooth, world.getDeltaTimeSmooth());
	}
}
