#pragma once

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Common.hpp>
#include <Engine/Net/IPv4Address.hpp>

namespace Engine::Net {
	// TODO: Doc
	bool startup();

	// TODO: Doc
	bool shutdown();

	// TODO: Doc
	IPv4Address hostToAddress(const std::string& uri);

	// TODO: Doc
	ENGINE_INLINE constexpr SequenceNumber seqToIndex(SequenceNumber seq);
}

#include <Engine/Net/Net.ipp>
