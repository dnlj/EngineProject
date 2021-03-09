#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ImGui/ImGui.hpp>
#include <Engine/FlatHashMap.hpp>
#include <imgui_node_editor.h>

// Game
#include <Game/Common.hpp>


namespace Game {
	class MapTestUI {
		public:
			/**
			 * Bit layout of ids.
			 * Ids are divided into two 32 bit node-pin pairs.
			 * If both pairs are present then this is a link id.
			 * If only the lower pair is present then this is a pin id.
			 * If only the lower node id is present then this is a node id.
			 * If only the pin part of either pair is present then this is an invalid id.
			 * If any node/pin/link id is zero it is an invalid/none id. All id sequences begin at one.
			 * All pins on a node, both input and ouput, share the same id sequence.
			 *
			 * 
			 *  Second Pin Id     Second Node Id    First Pin Id      First Node Id
			 * |---------------| |---------------| |---------------| |---------------|
			 * 00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000
			 * |---------------------------------| |---------------------------------|
			 *	 Second node-pin pair                First node-pin pair
			 *
			 */
			union PinId {
				uint32 full;
				struct {
					uint16 node;
					uint16 pin;
				};
				PinId() : full{} {}
				PinId(const ax::NodeEditor::PinId& id) : full{static_cast<uint32>(id.Get())} {}
				operator uint64() const { return full; }
				operator ax::NodeEditor::PinId() const { return full; }
			}; static_assert(sizeof(PinId) == 4, "This type is assumed to be tightly packed.");

			union Id {
				uint64 full;
				uint16 node;
				struct {
					PinId pin1;
					PinId pin2;
				};

				Id() : full{} {}
				Id(uint64 id) : full{id} {}
				Id(const ax::NodeEditor::NodeId& id) : full{id.Get()} {}
				Id(const ax::NodeEditor::LinkId& id) : full{id.Get()} {}
				Id(const ax::NodeEditor::PinId& id) : full{id.Get()} {}
				Id(const ax::NodeEditor::PinId& p1, const ax::NodeEditor::PinId& p2) : pin1{p1.Get()}, pin2{p2.Get()} {}

				operator ax::NodeEditor::NodeId() const { return full; }
				operator ax::NodeEditor::LinkId() const { return full; }
				operator ax::NodeEditor::PinId() const { return full; }

				operator uint64() const { return full; }

				std::string string() const { return "[("+ std::to_string(pin2.pin) + ", "+ std::to_string(pin2.node) + "), (" + std::to_string(pin1.pin) + ", "+ std::to_string(pin1.node) + ")]"; }

				struct Hash {
					ENGINE_INLINE size_t operator()(const Id& id) const noexcept {
						return static_cast<size_t>(id);
					}
				};
			}; static_assert(sizeof(Id) == 8, "This type is assumed to be tightly packed.");

			struct Pin {
				bool output = false;
			};

			struct Node {
				// Input and output pins share the same id sequence
				std::vector<Pin> pins;
				std::vector<Id> links;
			};

			struct Link {
			};

		private:
			ax::NodeEditor::EditorContext* ctx;
			Id lastNodeId = 0;

			Engine::FlatHashMap<Id, Link, Id::Hash> links;
			Engine::FlatHashMap<Id, Node, Id::Hash> nodes;

		public:
			MapTestUI();
			~MapTestUI();
			void generate();
			void render();
	};
}
