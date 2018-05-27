#pragma once
// STD
#include <iostream>

// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/FatalException.hpp>

#define ENGINE_LOG(msg)\
	Engine::Detail::log(std::clog, "[LOG]", __FILE__, __LINE__) << msg << '\n'

#define ENGINE_WARN(msg)\
	Engine::Detail::log(std::cerr, "[WARN]", __FILE__, __LINE__) << msg << '\n'

#define ENGINE_ERROR(msg)\
	Engine::Detail::log(std::cerr, "[ERROR]", __FILE__, __LINE__) << msg << '\n';\
	throw Engine::FatalException{};
