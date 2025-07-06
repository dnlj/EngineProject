#pragma once


namespace Game::Terrain::Layer {
	//
	//
	//
	// TODO: static assert layer dependency order
	//
	//
	//
	
	// TODO: incorporate this at the terrain level to verify the correct requests are made and avoid cycles.
	template<class...>
	class DependsOn{};
}
