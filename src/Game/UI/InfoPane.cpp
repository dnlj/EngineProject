// Game
#include <Game/UI/InfoPane.hpp>
#include <Game/EngineInstance.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/systems/NetworkingSystem.hpp>


namespace Game::UI {
	InfoPane::InfoPane(EUI::Context* context) : AutoList{context} {
		setTitle("Info");
		addLabel("FPS: {:.3f} ({:.6f})");
		addLabel("Tick: {}");
		addLabel("Tick Scale: {:.3f}");

		disconnect = ctx->constructPanel<EUI::Button>();
		disconnect->autoText("Disconnect");
		disconnect->lockSize();
		getContent()->addChild(disconnect);

		disconnect->setAction([](EUI::Button* btn){
			auto& world = btn->getContext()->getUserdata<EngineInstance>()->getWorld();

			for (const auto& ent : world.getFilter<ConnectionComponent>()) {
				const auto& addr = world.getComponent<ConnectionComponent>(ent).conn->address();
				world.getSystem<NetworkingSystem>().requestDisconnect(addr);
			}
		});

		ctx->addPanelUpdateFunc(getContent(), [](auto* panel){
			auto* self = reinterpret_cast<InfoPane*>(panel->getParent());
			self->update();
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
	}
}
