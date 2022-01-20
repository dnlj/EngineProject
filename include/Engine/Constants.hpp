#pragma once

// Engine
#include <Engine/ASCIIColorString.hpp>


namespace Engine::Constants { // TODO: C++20: namespace Engine::inline Constants
	template<class T>
	constexpr inline T Pi = static_cast<T>(3.141592653589793238462643383279502884197169);

	template<class T>
	constexpr inline T Sqrt2 = static_cast<T>(1.41421356237309504880168872420969807);

	template<class T>
	constexpr inline T InvSqrt2 = static_cast<T>(0.707106781186547524400844362104849039284);

	template<class T>
	constexpr inline T Phi = static_cast<T>(1.618033988749894848204586834365638117720309);

	template<class T>
	constexpr inline T InvPhi = static_cast<T>(0.618033988749894848204586834365638117720309);

	// TODO: move into own ns?
	constexpr inline ASCIIColorString ASCII_BLACK        = "\033[30m";
	constexpr inline ASCIIColorString ASCII_RED          = "\033[31m";
	constexpr inline ASCIIColorString ASCII_GREEN        = "\033[32m";
	constexpr inline ASCIIColorString ASCII_YELLOW       = "\033[33m";
	constexpr inline ASCIIColorString ASCII_BLUE         = "\033[34m";
	constexpr inline ASCIIColorString ASCII_MAGENTA      = "\033[35m";
	constexpr inline ASCIIColorString ASCII_CYAN         = "\033[36m";
	constexpr inline ASCIIColorString ASCII_WHITE        = "\033[37m";
	constexpr inline ASCIIColorString ASCII_BLACK_BOLD   = "\033[1;30m";
	constexpr inline ASCIIColorString ASCII_RED_BOLD     = "\033[1;31m";
	constexpr inline ASCIIColorString ASCII_GREEN_BOLD   = "\033[1;32m";
	constexpr inline ASCIIColorString ASCII_YELLOW_BOLD  = "\033[1;33m";
	constexpr inline ASCIIColorString ASCII_BLUE_BOLD    = "\033[1;34m";
	constexpr inline ASCIIColorString ASCII_MAGENTA_BOLD = "\033[1;35m";
	constexpr inline ASCIIColorString ASCII_CYAN_BOLD    = "\033[1;36m";
	constexpr inline ASCIIColorString ASCII_WHITE_BOLD   = "\033[1;37m";
	constexpr inline ASCIIColorString ASCII_RESET        = "\033[0m";

	constexpr inline ASCIIColorString ASCII_INFO         = ASCII_BLUE;
	constexpr inline ASCIIColorString ASCII_SUCCESS      = ASCII_GREEN;
	constexpr inline ASCIIColorString ASCII_WARN         = ASCII_YELLOW;
	constexpr inline ASCIIColorString ASCII_ERROR        = ASCII_RED;
	constexpr inline ASCIIColorString ASCII_FG           = ASCII_WHITE;
	constexpr inline ASCIIColorString ASCII_FG2          = ASCII_BLACK_BOLD;

}
namespace Engine { using namespace Engine::Constants; }
