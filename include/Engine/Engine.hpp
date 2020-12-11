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
	constexpr bool ENGINE_DEBUG = true;
#else
	constexpr bool ENGINE_DEBUG = false;
#endif

// TODO: move all this macro stuff into files?

// TODO: cross platform inline
#define ENGINE_INLINE __forceinline

// TODO: replace macros with source_location?
#define _ENGINE_CREATE_LOG_LAMBDA(Prefix, Color, Other)\
	([](auto&&... args){\
		::Engine::Detail::log(Prefix, Color,\
			(__FILE__ + sizeof(ENGINE_BASE_PATH)), __LINE__,\
			std::forward<decltype(args)>(args) ...\
		);\
		Other;\
	})

#define _ENGINE_CREATE_ASSERT_LAMBDA(Prefix, Color, Other)\
	([](bool cond, auto&&... args){\
		if (!cond) {\
			_ENGINE_CREATE_LOG_LAMBDA(Prefix, Color, Other)(std::forward<decltype(args)>(args)...);\
		}\
	})

#ifdef DEBUG
	#define ENGINE_DIE __debugbreak(); ::std::terminate();
#else
	#define ENGINE_DIE ::std::terminate();
#endif

#define ENGINE_LOG _ENGINE_CREATE_LOG_LAMBDA("[LOG]", Engine::ASCII_FG2, 0)
#define ENGINE_INFO _ENGINE_CREATE_LOG_LAMBDA("[INFO]", Engine::ASCII_INFO, 0)
#define ENGINE_SUCCESS _ENGINE_CREATE_LOG_LAMBDA("[SUCCESS]", Engine::ASCII_SUCCESS, 0)
#define ENGINE_WARN _ENGINE_CREATE_LOG_LAMBDA("[WARN]", Engine::ASCII_WARN, 0)
#define ENGINE_ERROR _ENGINE_CREATE_LOG_LAMBDA("[ERROR]", Engine::ASCII_ERROR, ENGINE_DIE)

#define ENGINE_ASSERT _ENGINE_CREATE_ASSERT_LAMBDA("[ERROR]", Engine::ASCII_ERROR, ENGINE_DIE)
#define ENGINE_ASSERT_WARN _ENGINE_CREATE_ASSERT_LAMBDA("[WARN]", Engine::ASCII_WARN, 0)

#if defined(DEBUG)
	#define ENGINE_DEBUG_ASSERT ENGINE_ASSERT
#else
	#define ENGINE_DEBUG_ASSERT(...)
#endif
