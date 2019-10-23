#pragma once

// Engine
#include <Engine/BindPressListener.hpp>

// Game
#include <Game/MapSystem.hpp>


namespace Game {
	template<MapChunk::EditMemberFunction func>
	class MapSystemBindListener : public Engine::BindPressListener, public Engine::BindHoldListener {
		public:
			MapSystemBindListener(MapSystem& mapSystem) : mapSystem{mapSystem} {};

		private:
			MapSystem& mapSystem;

			virtual void onBindPress() override {
				mapSystem.applyEdit<func>();
			};

			virtual void onBindHold() override {
				mapSystem.applyEdit<func>();
			};
	};
}
