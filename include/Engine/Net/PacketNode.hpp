#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Packet.hpp>


namespace Engine::Net {
	class PacketNode {
		public:
			PacketNode() = default;
			PacketNode(const PacketNode&) = delete; // Would break curr/last ptrs
			std::unique_ptr<PacketNode> next = nullptr;
			byte* curr;
			byte* last;

			Packet packet;

			void clear() {
				curr = packet.body;
				last = curr;
			}

			int32 size() { return static_cast<int32>(last - curr); }
	};
}
