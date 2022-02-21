#pragma once


namespace Engine::Net {
	/**
	 * TODO: doc, in short these are options that need to be set at the time of socket creation
	 */
	struct SocketFlag_ { enum SocketFlag {
		None = 0,
		NonBlocking = 1 << 0,
		ReuseAddress = 1 << 1,
	};};
	using SocketFlag = SocketFlag_::SocketFlag;
	ENGINE_BUILD_ALL_OPS(SocketFlag);
}
