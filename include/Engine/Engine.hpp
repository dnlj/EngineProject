#pragma once


// STD
#include <cstdint>
#include <iostream>
#include <limits>

// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/FatalException.hpp>

// TODO: make these usable in a noexcept context

#define ENGINE_LOG(msg)\
	Engine::Detail::log(std::clog, "[LOG]", __FILE__, __LINE__) << msg << '\n'

#define ENGINE_WARN(msg)\
	Engine::Detail::log(std::cerr, "[WARN]", __FILE__, __LINE__) << msg << '\n'

#define ENGINE_ERROR(msg)\
	Engine::Detail::log(std::cerr, "[ERROR]", __FILE__, __LINE__) << msg << '\n';\
	throw Engine::FatalException{};

// TODO: test
#if defined(DEBUG)
	// TODO: insert cond in message
	#define ENGINE_ASSERT(cond, msg) if (!(cond)) { ENGINE_ERROR("Assertion failed: " << msg); }
#else
	#define ENGINE_ASSERT(cond, msg)
#endif

// TODO: move to own file
namespace Engine::Types {
	static_assert(std::numeric_limits<char>::digits + std::numeric_limits<char>::is_signed == 8, "This program assumes an 8 bit byte.");

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

namespace Engine {
	using namespace Engine::Types;
}

// TODO: is this a good place for these?
namespace Engine {
	constexpr float32 PI = 3.141592653589793238462643383279502884197169f;
}
