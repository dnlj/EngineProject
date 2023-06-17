#pragma once

// To Compare: Visual Studio > Tools > Options > VC Project Settings > Build Timing
#if 1

// STD
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <source_location>


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
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

// GLM
#pragma warning (push)
#pragma warning (disable: 4701)
#include <glm/glm.hpp>
#pragma warning (pop)
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Hash.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/RingBuffer.hpp>
#include <Engine/SparseSet.hpp>

// Game
// This does speed up compile by a few seconds but it also would mean much more
// frequent rebuild of the pch
#include <Game/World.hpp>

#endif
