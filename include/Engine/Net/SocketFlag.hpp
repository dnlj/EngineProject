#pragma once


namespace Engine::Net {
	/**
	 * Socket flags are options which must be set at the time of socket creation.
	 * As opposed to socket options, which can be set after creation (binding).
	 *
	 * @see SocketOption
	 */
	struct SocketFlag_ { enum SocketFlag {
		None = 0,
		NonBlocking = 1 << 0,
		ReuseAddress = 1 << 1,
	};};
	using SocketFlag = SocketFlag_::SocketFlag;
	ENGINE_BUILD_ALL_OPS(SocketFlag);
}
