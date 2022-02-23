#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Hash.hpp>
#include <Engine/Input/InputType.hpp>
#include <Engine/Input/KeyCode.hpp>


// TODO: Doc
// TODO: split
namespace Engine::Input {
	using DeviceId = uint8;

	class InputId {
		public:
			InputType type = {};
			DeviceId device = 0;
			uint16 code = 0;

			constexpr bool isAxis() const {
				return isAxisInput(type);
			}

			constexpr operator bool() const {
				return static_cast<bool>(type);
			}

			friend bool operator==(const InputId& first, const InputId& second) {
				return first.code == second.code
					&& first.type == second.type
					&& first.device == second.device;
			};
	};
	static_assert(sizeof(InputId) == 4);
}

namespace Engine {
	template<>
	struct Hash<Input::InputId> {
		size_t operator()(const Input::InputId& v) const {
			static_assert(sizeof(v) == sizeof(uint32));
			//auto seed = hash(v.type);
			//hashCombine(seed, hash(v.code));
			//hashCombine(seed, hash(v.device));
			//return seed;
			return reinterpret_cast<const uint32&>(v);
		}
	};
	
	template<> struct LogFormatter<Input::InputId> {
		static void format(class ::std::ostream& stream, const Input::InputId& val);
	};
}
