#pragma once

// STD
#include <limits>
#include <stdint.h>


namespace Engine::Types { // TODO: C++20: namespace Engine::inline Types {
	static_assert(std::numeric_limits<char>::digits + std::numeric_limits<char>::is_signed == 8, "This program assumes an 8 bit byte.");

	// TODO: index or size type would be useful

	using byte = unsigned char; // Must be "unsigned char" as specified in the C++ standard section "intro.object"
	using uint = unsigned int;

	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using intf8 = int_fast8_t;
	using intf16 = int_fast16_t;
	using intf32 = int_fast32_t;
	using intf64 = int_fast64_t;

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using uintf8 = uint_fast8_t;
	using uintf16 = uint_fast16_t;
	using uintf32 = uint_fast32_t;
	using uintf64 = uint_fast64_t;

	using uintz = size_t;
	using intz = ptrdiff_t;

	using float32 = float;
	static_assert(sizeof(float32) == 4, "float32 is an incorrect number of bytes.");
	static_assert(std::numeric_limits<float32>::is_iec559, "float32 is not IEEE 754 single-precision (binary32) format.");

	using float64 = double;
	static_assert(sizeof(float64) == 8, "float64 is an incorrect number of bytes.");
	static_assert(std::numeric_limits<float64>::is_iec559, "float64 is not IEEE 754 double-precision (binary64) format.");
}
namespace Engine { using namespace Engine::Types; }
