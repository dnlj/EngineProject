#pragma once

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Common.hpp>
#include <Engine/Net/IPv4Address.hpp>

namespace Engine::Net {
	constexpr static uint16 protocol = 0b0'0110'1001'1001'0110;

	// TODO: Doc
	bool startup();

	// TODO: Doc
	bool shutdown();

	// TODO: Doc
	IPv4Address hostToAddress(const std::string& uri);
}
