#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Hash.hpp>
#include <Engine/Input/InputType.hpp>


// TODO: Doc
// TODO: split
namespace Engine::Input {
	// TODO: This cannot distinguish between two devices
	class InputId {
		public:
			InputType type = InputType::UNKNOWN;
			int16 code = 0;

			constexpr bool isAxis() const {
				return isAxisInput(type);
			}

			constexpr operator bool() const {
				return static_cast<bool>(type);
			}

			friend bool operator==(const InputId& first, const InputId& second) {
				return first.code == second.code
					&& first.type == second.type;
			};
	};
}

namespace Engine {
	template<>
	struct Hash<Input::InputId> {
		size_t operator()(const Input::InputId& v) const {
			auto seed = hash(v.type);
			hashCombine(seed, hash(v.code));
			return seed;
		}
	};
}
