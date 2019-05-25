#pragma once

// STD
#include <cstdint>
#include <array>

// Engine
#include <Engine/Hash.hpp>


// TODO: Doc
// TODO: split
namespace Engine {
	// TODO: move
	enum class InputType : int8_t {
		UNKNOWN = 0,
		KEYBOARD = 1,
		MOUSE = 2,
		GAMEPAD = 3,
	};

	class Input {
		public:
			InputType type = InputType::UNKNOWN;
			int code = 0;

			operator bool() const {
				return static_cast<bool>(type);
			}

			// TODO: split
			friend bool operator==(const Input& first, const Input& second) {
				return first.code == second.code
					&& first.type == second.type;
			};
	};

	template<>
	class Hash<Input> {
		public:
			size_t operator()(const Input& input) const {
				using InputTypeUnder = std::underlying_type_t<decltype(Input::type)>;
				Hash<InputTypeUnder> a;
				Hash<decltype(Input::code)> b;
				auto seed = a(static_cast<InputTypeUnder>(input.type));
				hashCombine(seed, b(input.code));
				return seed;
			}
	};
	
	struct InputState {
		Input input{};
		bool state = false; // TODO: will need to generalize this for more than just buttons (e.g. an axis)
	};

	using InputSequence = std::array<Input, 4>;
}
