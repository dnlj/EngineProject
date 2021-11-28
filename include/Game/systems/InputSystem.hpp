#pragma once

// STD
#include <functional>
#include <limits>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/Input/Value.hpp>
#include <Engine/RingBuffer.hpp>

// Game
#include <Game/System.hpp>

namespace Game {
	class InputSystem : public System {
		private:
			using Command = uint32;
			using Func = std::function<void(Engine::Input::Value)>;

			class Event {
				public:
					Command command;
					Engine::Clock::TimePoint time;
					Engine::Input::Value value;
			};

			std::vector<Func> commands;

			template<class C>
			constexpr static Command validateCommandType(C cmd) {
				using A = std::numeric_limits<std::underlying_type_t<C>>;
				using B = std::numeric_limits<Command>;
				static_assert(
					   (A::max() <= B::max())
					&& (A::min() >= B::min())
					&& (A::is_integer)
				);
				return static_cast<Command>(cmd);
			};

		public:
			InputSystem(SystemArg arg);
			void tick();

			template<class C>
			void pushEvent(C cmd, Engine::Clock::TimePoint time, Engine::Input::Value value) {
				buffer.push({
					.command = validateCommandType(cmd),
					.time = time,
					.value = value,
				});
			}

			template<class C>
			ENGINE_INLINE void registerCommand(C command, Func func) {
				auto cmd = validateCommandType(command);

				if (cmd >= commands.size()) {
					commands.resize(cmd + 1);
				}

				commands[cmd] = func;
			}
			
			template<class C>
			ENGINE_INLINE void deregisterCommand(C command) {
				auto cmd = validateCommandType(command);

				if (cmd < commands.size()) {
					commands[cmd] = nullptr;
				} else [[unlikely]] {
					ENGINE_WARN("Attempting to deregister and unregistered input command. Ignoring.");
				}
			}

		private:
			Engine::StaticRingBuffer<Event, 128> buffer;
	};
}
