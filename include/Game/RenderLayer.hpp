#pragma once


namespace Game {
	enum class RenderLayer {
		Parallax_Background,
		Background,
		Terrain,
		Main,
		Foreground,
		UserInterface,

		#ifdef DEBUG_PHYSICS
		PhysicsDebug,
		#endif

		_count,
	};
	ENGINE_BUILD_ALL_OPS(RenderLayer)
}
