#pragma once

// To Compare: Visual Studio > Tools > Options > VC Project Settings > Build Timing
#if 1

// STD
#include <iosfwd>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <limits>
#include <numeric>
#include <csignal>
#include <functional>
#include <memory>
#include <cmath>
#include <cassert>
#include <utility>
#include <initializer_list>
#include <tuple>


// Windows
#if ENGINE_OS_WINDOWS
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <io.h>
#endif

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// FMT
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/Hash.hpp>
#include <Engine/RingBuffer.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/SparseSet.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/ECS/World.hpp>

// Game
// This does speed up compile by a few seconds but it also would mean much more
// frequent rebuild of the pch
#include <Game/World.hpp>

#endif
