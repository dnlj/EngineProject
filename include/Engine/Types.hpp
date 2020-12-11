#pragma once

namespace Engine::Types { // TODO: C++20: namespace Engine::inline Types {
	static_assert(std::numeric_limits<char>::digits + std::numeric_limits<char>::is_signed == 8, "This program assumes an 8 bit byte.");

	// TODO: index or size type would be useful

	using byte = unsigned char;
	// TODO: enum byte : uint8_t {};

	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using float32 = float;
	static_assert(sizeof(float32) == 4, "float32 is an incorrect number of bytes.");
	static_assert(std::numeric_limits<float32>::is_iec559, "float32 is not IEEE 754 single-precision (binary32) format.");

	using float64 = double;
	static_assert(sizeof(float64) == 8, "float64 is an incorrect number of bytes.");
	static_assert(std::numeric_limits<float64>::is_iec559, "float64 is not IEEE 754 double-precision (binary64) format.");
}
namespace Engine { using namespace Engine::Types; }
