#pragma once


namespace Game {
	enum class RenderLayer {
		Background,
		Default,
		Foreground,
		_COUNT,
	};
	ENGINE_BUILD_ALL_OPS(RenderLayer)
}
