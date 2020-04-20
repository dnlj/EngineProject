#pragma once

// STD
#include <concepts>

// Engine
#include <Engine/ImGui/ImGui.hpp>


template<class T>
ImVec2 operator*(const T& a, const ImVec2& b) {
	return {
		static_cast<decltype(b.x)>(a) * b.x,
		static_cast<decltype(b.y)>(a) * b.y
	};
}

template<class T>
inline ImVec2 operator*(const ImVec2& a, const T& b) {
	return b * a;
}
