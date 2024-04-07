#pragma once

// Game
#include <Game/ConnectionInfo.hpp>

namespace Game {
	class NetworkComponent {
		public:
			NetworkComponent(ConnectionInfo& conn) : conn{&conn} {}

			ConnectionInfo& get() const noexcept {
				ENGINE_DEBUG_ASSERT(conn != nullptr, "Connection info is null. This should not happen.");
				return *conn;
			}

		private:
			ConnectionInfo* conn;
	};
}
