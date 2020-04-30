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

	// TODO: move into own ns?
	constexpr std::string_view ASCII_BLACK        = "\033[30m";
	constexpr std::string_view ASCII_RED          = "\033[31m";
	constexpr std::string_view ASCII_GREEN        = "\033[32m";
	constexpr std::string_view ASCII_YELLOW       = "\033[33m";
	constexpr std::string_view ASCII_BLUE         = "\033[34m";
	constexpr std::string_view ASCII_MAGENTA      = "\033[35m";
	constexpr std::string_view ASCII_CYAN         = "\033[36m";
	constexpr std::string_view ASCII_WHITE        = "\033[37m";
	constexpr std::string_view ASCII_BLACK_BOLD   = "\033[1;30m";
	constexpr std::string_view ASCII_RED_BOLD     = "\033[1;31m";
	constexpr std::string_view ASCII_GREEN_BOLD   = "\033[1;32m";
	constexpr std::string_view ASCII_YELLOW_BOLD  = "\033[1;33m";
	constexpr std::string_view ASCII_BLUE_BOLD    = "\033[1;34m";
	constexpr std::string_view ASCII_MAGENTA_BOLD = "\033[1;35m";
	constexpr std::string_view ASCII_CYAN_BOLD    = "\033[1;36m";
	constexpr std::string_view ASCII_WHITE_BOLD   = "\033[1;37m";
	constexpr std::string_view ASCII_RESET        = "\033[0m";

	constexpr std::string_view ASCII_INFO     = ASCII_BLUE;
	constexpr std::string_view ASCII_SUCCESS  = ASCII_GREEN;
	constexpr std::string_view ASCII_WARN     = ASCII_YELLOW;
	constexpr std::string_view ASCII_ERROR    = ASCII_RED;
	constexpr std::string_view ASCII_FG       = ASCII_WHITE;
	constexpr std::string_view ASCII_FG2      = ASCII_BLACK_BOLD;

}
namespace Engine { using namespace Engine::Constants; }


#define ENGINE_SIDE_SERVER 1
#define ENGINE_SIDE_CLIENT 2
#define ENGINE_SERVER (ENGINE_SIDE == ENGINE_SIDE_SERVER)
#define ENGINE_CLIENT (ENGINE_SIDE == ENGINE_SIDE_CLIENT)

#ifdef DEBUG
	constexpr bool ENGINE_DEBUG = true;
#else
	constexpr bool ENGINE_DEBUG = false;
#endif

// TODO: move all this macro stuff into files?

// TODO: cross platform inline
#define ENGINE_INLINE __forceinline

// TODO: replace macros with source_location?
#define _ENGINE_CREATE_LOG_LAMBDA(File, Prefix, Color, Other)\
	([](auto&&... args){\
		::Engine::Detail::log(File, Prefix, Color,\
			(__FILE__ + sizeof(ENGINE_BASE_PATH)), __LINE__,\
			std::forward<decltype(args)>(args) ...\
		);\
		Other;\
	})

#define _ENGINE_CREATE_ASSERT_LAMBDA(File, Prefix, Color, Other)\
	([](bool cond, auto&&... args){\
		if (!cond) {\
			_ENGINE_CREATE_LOG_LAMBDA(File, Prefix, Color, Other)(std::forward<decltype(args)>(args)...);\
		}\
	})

#define ENGINE_DIE ::std::terminate();

#define ENGINE_LOG _ENGINE_CREATE_LOG_LAMBDA(stdout, "[LOG]", Engine::ASCII_FG2, 0)
#define ENGINE_INFO _ENGINE_CREATE_LOG_LAMBDA(stdout, "[INFO]", Engine::ASCII_INFO, 0)
#define ENGINE_SUCCESS _ENGINE_CREATE_LOG_LAMBDA(stdout, "[SUCCESS]", Engine::ASCII_SUCCESS, 0)
#define ENGINE_WARN _ENGINE_CREATE_LOG_LAMBDA(stderr, "[WARN]", Engine::ASCII_WARN, 0)
#define ENGINE_ERROR _ENGINE_CREATE_LOG_LAMBDA(stderr, "[ERROR]", Engine::ASCII_ERROR, ENGINE_DIE)

#define ENGINE_ASSERT _ENGINE_CREATE_ASSERT_LAMBDA(stderr, "[ERROR]", Engine::ASCII_ERROR, ENGINE_DIE)
#define ENGINE_ASSERT_WARN _ENGINE_CREATE_ASSERT_LAMBDA(stderr, "[WARN]", Engine::ASCII_WARN, 0)

#if defined(DEBUG)
	#define ENGINE_DEBUG_ASSERT ENGINE_ASSERT
#else
	#define ENGINE_DEBUG_ASSERT(...)
#endif
