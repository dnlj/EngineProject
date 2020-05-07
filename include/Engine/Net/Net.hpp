#pragma once

// STD
#include <string>

// Engine
#include <Engine/Net/IPv4Address.hpp>


namespace Engine::Net {
	// TODO: Doc
	bool startup();

	// TODO: Doc
	bool shutdown();

	IPv4Address hostToAddress(const std::string& uri);
}
