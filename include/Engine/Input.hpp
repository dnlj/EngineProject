#pragma once

// Engine
#include <Engine/Hash.hpp>
#include <Engine/InputType.hpp>


// TODO: Doc
// TODO: split
namespace Engine {
	class Input {
		public:
			InputType type = InputType::UNKNOWN;
			int code = 0;

			operator bool() const {
				return static_cast<bool>(type);
			}

			friend bool operator==(const Input& first, const Input& second) {
				return first.code == second.code
					&& first.type == second.type;
			};
	};

	template<>
	class Hash<Input> {
		public:
			size_t operator()(const Input& input) const {
				Hash<decltype(Input::type)> a;
				Hash<decltype(Input::code)> b;
				auto seed = a(input.type);
				hashCombine(seed, b(input.code));
				return seed;
			}
	};
}
