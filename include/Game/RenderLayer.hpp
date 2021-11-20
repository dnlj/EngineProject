#pragma once


namespace Game {
	// TODO: move to struct>enum>using style
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
