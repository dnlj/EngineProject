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
#include <Engine/GlobalConfig.hpp>
#include <Engine/FatalException.hpp>
#include <Engine/Constants.hpp>
#include <Engine/Types.hpp>

#define ENGINE_SIDE_SERVER 1
#define ENGINE_SIDE_CLIENT 2
#define ENGINE_SIDE_BOTH (ENGINE_SIDE_SERVER | ENGINE_SIDE_CLIENT)
#define ENGINE_SERVER (ENGINE_SIDE == ENGINE_SIDE_SERVER)
#define ENGINE_CLIENT (ENGINE_SIDE == ENGINE_SIDE_CLIENT)

#ifdef DEBUG
	#define ENGINE_DEBUG true
#else
	#define ENGINE_DEBUG false
#endif

// TODO: look into other inline options
// GCC: https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
// CLANG: https://clang.llvm.org/docs/AttributeReference.html
// MSVC: https://www.reddit.com/r/programming/comments/g9inos/smarter_cc_inlining_with_attribute_flatten/foyoq62/
// 
// We have always had __forceinline, and I just added four attributes in this area,
// 
//     [[msvc::forceinline]] - same as __forceinline on a function definition),
//     [[msvc::noinline]] - same as declspec(noinline) on a function,
//     [[msvc::forceinline_calls]] - goes on statements or statement blocks, and attempts to forceinline all calls in that block,
//     [[msvc::noinline_calls]] - don't inline any function calls in this block
// 
// Nothing like flatten (msvc::forceinline_calls isn't recursive, it only does "one level"), although that would be easy to implement
//

/**
 * Attempts to force a function to be inlined.
 */
#define ENGINE_INLINE [[msvc::forceinline]] // TODO: cross platform: [[gnu::always_inline]]

/**
 * Attempts to force inline all function calls in a block or statement.
 */
#define ENGINE_INLINE_CALLS [[msvc::forceinline_calls]] // TODO: cross platform: [[gnu::flatten]]



// TODO: replace macros with source_location?
#define _ENGINE_CREATE_LOG_LAMBDA(Prefix, Decorate, Color, Other)\
	([](auto&&... args){\
		::Engine::Detail::log<Decorate>(Prefix, Color,\
			(__FILE__ + sizeof(ENGINE_BASE_PATH)), __LINE__,\
			std::forward<decltype(args)>(args) ...\
		);\
		Other;\
	})

#define _ENGINE_CREATE_ASSERT_LAMBDA(Prefix, Decorate, Color, Other)\
	([](auto cond, auto&&... args){\
		if (!cond) {\
			_ENGINE_CREATE_LOG_LAMBDA(Prefix, Decorate, Color, Other)(std::forward<decltype(args)>(args)...);\
		}\
	})

#if ENGINE_DEBUG
	#define ENGINE_DIE __debugbreak(); ::std::terminate();
#else
	#define ENGINE_DIE ::std::terminate();
#endif
	
#define ENGINE_RAW_TEXT _ENGINE_CREATE_LOG_LAMBDA("[TEXT]", false, Engine::ASCII_FG2, 0)
#define ENGINE_LOG _ENGINE_CREATE_LOG_LAMBDA("[LOG]", true, Engine::ASCII_FG2, 0)
#define ENGINE_INFO _ENGINE_CREATE_LOG_LAMBDA("[INFO]", true, Engine::ASCII_INFO, 0)
#define ENGINE_SUCCESS _ENGINE_CREATE_LOG_LAMBDA("[SUCCESS]", true, Engine::ASCII_SUCCESS, 0)
#define ENGINE_WARN _ENGINE_CREATE_LOG_LAMBDA("[WARN]", true, Engine::ASCII_WARN, 0)
#define ENGINE_FAIL _ENGINE_CREATE_LOG_LAMBDA("[FAIL]", true, Engine::ASCII_ERROR, 0)
#define ENGINE_ERROR _ENGINE_CREATE_LOG_LAMBDA("[ERROR]", true, Engine::ASCII_ERROR, ENGINE_DIE)

#define ENGINE_ASSERT _ENGINE_CREATE_ASSERT_LAMBDA("[ERROR][ASSERT]", true, Engine::ASCII_ERROR, ENGINE_DIE)
#define ENGINE_ASSERT_WARN _ENGINE_CREATE_ASSERT_LAMBDA("[WARN][ASSERT]", true, Engine::ASCII_WARN, 0)

#if ENGINE_DEBUG
	#define ENGINE_DEBUG_ASSERT ENGINE_ASSERT
#else
	#define ENGINE_DEBUG_ASSERT(...)
#endif
