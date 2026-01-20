#pragma once


namespace Game::Terrain::Layer {
	// TODO: Incorporate this at the terrain level to verify the correct requests are made and avoid
	//       cycles. This probably wouldn't be to difficult to add. We know the currently layer
	//       being processed so it would just be a matter of layer inheriting from this and then
	//       checking indices I think.
	template<class...>
	class DependsOn{};
}
