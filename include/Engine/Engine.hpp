#pragma once


// TODO: trim includes
// STD
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <ctime>

// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/FatalException.hpp>

namespace Engine::Types { // TODO: C++20: namespace Engine::inline Types {
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
namespace Engine { using namespace Engine::Types; }

namespace Engine::Constants { // TODO: C++20: namespace Engine::inline Constants
	constexpr float32 PI = 3.141592653589793238462643383279502884197169f;
	constexpr int32 ServerSide = 1 << 0;
	constexpr int32 ClientSide = 1 << 1;
}
namespace Engine { using namespace Engine::Constants; }


// TODO: rm - see Engine::Detail cpp file
namespace Engine::Detail {
	inline std::string getDateTimeString() {
		// Example output: 2017-12-24 18:29:35 -0600
		const auto time = std::time(nullptr);
		std::string date(26, '.');
		
		std::strftime(date.data(), date.size(), "%Y-%m-%d %H:%M:%S %z", localtime(&time));
		date.pop_back();
		return date;
	}
}

// TODO: move all this macro stuff into files?

#define ENGINE_SERVER (ENGINE_SIDE == ::Engine::ServerSide)
#define ENGINE_CLIENT (ENGINE_SIDE == ::Engine::ClientSide)

// TODO: replace macros with source_location?
#define _ENGINE_CREATE_LOG_LAMBDA(Stream, Prefix, Other)\
	([](auto&&... args){\
		Stream\
			<< "[" << ::Engine::Detail::getDateTimeString() << "]"\
			<< "[" << __FILE__ << ":" << __LINE__ << "]"\
			<< Prefix << " ";\
		(Stream << ... << std::forward<decltype(args)>(args));\
		Stream << '\n';\
		Other;\
	})

#define _ENGINE_CREATE_ASSERT_LAMBDA(Stream, Prefix, Other)\
	([](bool cond, auto&&... args){\
		if (!cond) {\
			_ENGINE_CREATE_LOG_LAMBDA(Stream, Prefix, Other)(std::forward<decltype(args)>(args)...);\
		}\
	})

#define ENGINE_DIE std::terminate();

#define ENGINE_LOG _ENGINE_CREATE_LOG_LAMBDA(::std::cout, "[LOG]", 0)
#define ENGINE_WARN _ENGINE_CREATE_LOG_LAMBDA(::std::cerr, "[WARN]", 0)
#define ENGINE_ERROR _ENGINE_CREATE_LOG_LAMBDA(::std::cerr, "[ERROR]", ENGINE_DIE)

#define ENGINE_ASSERT _ENGINE_CREATE_ASSERT_LAMBDA(::std::cerr, "[ERROR]", ENGINE_DIE)
#define ENGINE_ASSERT_WARN _ENGINE_CREATE_ASSERT_LAMBDA(::std::cerr, "[WARN]", nullptr)

#if defined(DEBUG)
	#define ENGINE_DEBUG_ASSERT ENGINE_ASSERT
#else
	#define ENGINE_DEBUG_ASSERT(...)
#endif
