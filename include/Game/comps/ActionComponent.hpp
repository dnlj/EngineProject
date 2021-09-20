#pragma once

// Engine
#include <Engine/ECS/Common.hpp>
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
		#if ENGINE_SERVER
			Engine::ECS::Tick recvTick;
		#else
			glm::vec2 screenTarget;
		#endif
			ActionValue buttons[static_cast<int32>(Action::_button_count)];
			glm::vec2 target;

			void netRead(Connection& conn) {
				for (auto& b : buttons) {
					// TODO: better interface for reading bits.
					b.pressCount = static_cast<decltype(b.pressCount)>(conn.read<2>());
					b.releaseCount = static_cast<decltype(b.releaseCount)>(conn.read<2>());
					b.latest = static_cast<decltype(b.latest)>(conn.read<1>());
				}

				const auto x = conn.read<32>();
				const auto y = conn.read<32>();
				target.x = reinterpret_cast<const float32&>(x);
				target.y = reinterpret_cast<const float32&>(y);
			}
	};

	class ActionComponent {
		private:
			friend class ActionSystem;
			Engine::SequenceBuffer<Engine::ECS::Tick, ActionState, tickrate> states;
			ActionState* state;

			// TODo: rename. use tickTrend
			public: float32 estBufferSize = 0.0f;

		public:
			constexpr static int32 maxStates = decltype(states)::capacity();

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
