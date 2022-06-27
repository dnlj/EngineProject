// Engine
#include <Engine/Net/UDPSocket.hpp> // Required for ENGINE_UDP_NETWORK_SIM - probably move to premake config.

// Game
#include <Game/UI/NetCondPane.hpp>
#include <Game/systems/NetworkingSystem.hpp>


namespace Game::UI {
	NetCondPane::NetCondPane(EUI::Context* context) : CollapsibleSection{context} {
		getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
		setTitle("Network Conditions");
				
		#ifndef ENGINE_UDP_NETWORK_SIM
			auto label = ctx->createPanel<EUI::Label>(getContent());
			label->autoText("Network simulation disabled.");
		#else
			addSlider("Half Ping Add").setLimits(0, 500).setValue(0).bind(
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					s.setValue(static_cast<float64>(std::chrono::duration_cast<std::chrono::milliseconds>(settings.halfPingAdd).count()));
				},
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					settings.halfPingAdd = std::chrono::milliseconds{static_cast<int64>(s.getValue())};
				}
			);
			addSlider("Jitter").setLimits(0, 1).setValue(0).bind(
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					s.setValue(settings.jitter);
				},
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					settings.jitter = static_cast<float32>(s.getValue());
				}
			);
			addSlider("Duplicate Chance").setLimits(0, 1).setValue(0).bind(
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					s.setValue(settings.duplicate);
				},
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					settings.duplicate = static_cast<float32>(s.getValue());
				}
			);
			addSlider("Loss").setLimits(0, 1).setValue(0).bind(
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					s.setValue(settings.loss);
				},
				[](EUI::Slider& s){
					auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
					auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
					settings.loss = static_cast<float32>(s.getValue());
				}
			);
		#endif
	}

	EUI::Slider& NetCondPane::addSlider(std::string_view txt) {
		auto label = ctx->constructPanel<EUI::Label>();
		label->autoText(txt);
		label->setWeight(1);

		auto slider = ctx->constructPanel<EUI::Slider>();
		slider->setWeight(2);
				
		auto line = ctx->createPanel<Panel>(getContent());
		line->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, ctx->getTheme().sizes.pad1});
		line->setAutoSizeHeight(true);
		line->addChildren({label, slider});
		return *slider;
	}
}
