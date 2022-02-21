#pragma once


namespace Engine::Net {
	struct SocketFlags_ { enum SocketFlags {
		None = 0,
		NonBlocking = 1 << 0,
	};};
	using SocketFlags = SocketFlags_::SocketFlags;
}
