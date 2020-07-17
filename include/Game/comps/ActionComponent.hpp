#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Input/Action.hpp>
#include <Engine/StaticRingBuffer.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class ButtonValue {
		public:
			uint8 pressCount;
			uint8 releaseCount;
			bool latest;
	};

	using AxisValue = float32;

	enum class Button : uint8 {
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		Attack1,
		Attack2,
		_COUNT,
	};

	enum class Axis : uint8 {
		TargetX,
		TargetY,
		_COUNT,
	};

	class ActionState {
		public:
			Engine::ECS::Tick recvTick;

			ButtonValue buttons[static_cast<int32>(Button::_COUNT)];

			// TODO: should be able to compress these quite a bit if we make them offsets from the player and as such have a limited range.
			AxisValue axes[static_cast<int32>(Axis::_COUNT)];
	};

	class ActionComponent {
		private:
			friend class ActionSystem;
			ActionState states[snapshots];
			ActionState state;

		public:
			const ButtonValue& getButton(Button btn) const {
				return state.buttons[static_cast<int32>(btn)];
			}

			const AxisValue& getAxis(Axis axis) const {
				return state.axes[static_cast<int32>(axis)];
			}
	};
}
