#pragma once

namespace Game {
	// Avoid including the actual type in this file since this is required in the base system in System.hpp
	class ConnectionInfo;

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
