#pragma once

// Engine
#include <Engine/ECS/ecs.hpp>
#include <Engine/SequenceBuffer.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/Connection.hpp>


namespace Game {
	class ActionValue {
		public:
			uint8 pressCount;
			uint8 releaseCount;
			bool latest;
	};

	enum class Action : uint8 {
		// Buttons
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		Attack1,
		Attack2,
		_button_count,

		// Axes
		Target = _button_count,
		_axis_count,

		_count = _button_count + _axis_count,
	};

	class ActionState {
		public:
			Engine::ECS::Tick recvTick; // TODO: server only
			glm::vec2 screenTarget; // TODO: client only

			ActionValue buttons[static_cast<int32>(Action::_button_count)];
			glm::vec2 target;

			void netRead(Engine::Net::BufferReader& msg) {
				for (auto& b : buttons) {
					msg.read<2>(&b.pressCount);
					msg.read<2>(&b.releaseCount);
					msg.read<1>(&b.latest);
				}

				const auto x = msg.read<32, uint32>();
				const auto y = msg.read<32, uint32>();
				target.x = reinterpret_cast<const float32&>(x);
				target.y = reinterpret_cast<const float32&>(y);
			}
	};

	class ActionComponent {
		public:
			Engine::SequenceBuffer<Engine::ECS::Tick, ActionState, tickrate> states; // TODO: why is this public?

		private:
			friend class ActionSystem; // TODO: rm - no longer needed
			ActionState* state = nullptr;

			// TODo: rename. use tickTrend
			public: float32 estBufferSize = 0.0f;

		public:
			constexpr static int32 maxStates = decltype(states)::capacity();

			ActionComponent(Engine::ECS::Tick initTick) : states{initTick} {}

			ENGINE_INLINE bool valid() const noexcept { return state != nullptr; }

			// TODO: are these called on server? nullptr check
			ENGINE_INLINE const ActionValue& getAction(Action btn) const {
				return state->buttons[static_cast<int32>(btn)];
			}

			ENGINE_INLINE const auto& getTarget() const {
				return state->target;
			}
	};
}
