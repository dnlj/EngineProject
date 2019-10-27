#pragma once

// Engine
#include <Engine/Hash.hpp>
#include <Engine/Input/InputType.hpp>


// TODO: Doc
// TODO: split
namespace Engine::Input { // TODO: change namespace name? I dont like having the same name for a class and namespace. InputId?
	class InputId {
		public:
			InputType type = InputType::UNKNOWN;
			int code = 0;

			operator bool() const {
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
