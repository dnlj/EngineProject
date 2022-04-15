#pragma once


namespace Game {
	enum class RenderLayer {
		Parallax_Background,
		Background,
		Terrain,
		Main,
		Foreground,

		#ifdef DEBUG_PHYSICS
		PhysicsDebug,
		#endif

		UserInterface,

		_count,
	};
	ENGINE_BUILD_ALL_OPS(RenderLayer)
}
