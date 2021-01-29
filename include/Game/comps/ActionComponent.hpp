#pragma once

// STD
#include <vector>
#include <iostream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Input/Action.hpp>
#include <Engine/SequenceBuffer.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/Connection.hpp>


namespace Game {
	class ButtonValue {
		public:
			uint8 pressCount;
			uint8 releaseCount;
			bool latest;
	};

	enum class Button : uint8 {
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		Attack1,
		Attack2,
		_COUNT,
	};

	class ActionState {
		public:
			Engine::ECS::Tick recvTick;
			ButtonValue buttons[static_cast<int32>(Button::_COUNT)];
			glm::vec2 screenTarget; // TODO: client only
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

			friend std::ostream& operator<<(std::ostream& os, const ActionState& s) {
				os << "ActionState(";
				for (const auto& b : s.buttons) {
					os
						<< " <" << static_cast<int>(b.pressCount)
						<< ", " << static_cast<int>(b.releaseCount)
						<< ", " << static_cast<int>(b.latest)
						<< ">";
				}
				// TODO: target
				os << " )";
				return os;
			}
	};

	class ActionComponent {
		private:
			friend class ActionSystem;
			//ActionState states[snapshots];
			// TODO: tick is currently signed. Why?? SequenceBuffer expects unsigned
			Engine::SequenceBuffer<Engine::ECS::Tick, ActionState, tickrate> states;
			ActionState* state;

			// TODo: rn. use tickTrend
			public: float32 estBufferSize = 0.0f;

		public:
			constexpr static int32 maxStates = decltype(states)::capacity();

			// TODO: are these called on server? nullptr check
			ENGINE_INLINE const ButtonValue& getButton(Button btn) const {
				return state->buttons[static_cast<int32>(btn)];
			}

			ENGINE_INLINE const auto& getTarget() const {
				return state->target;
			}
	};
}
