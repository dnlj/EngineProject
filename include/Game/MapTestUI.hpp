#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ImGui/ImGui.hpp>
#include <imgui_node_editor.h>
#include <Engine/FlatHashMap.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class MapTestUI {
		private:
			ax::NodeEditor::EditorContext* ctx;

			// 16 bits node id 8 bits input number 8 bits output number
			using Id = uint64;

			// TODO: rm - just make a union. way easier than getters and setters
			//constexpr static Id getNodePart(Id id) noexcept { return id & 0xFFFF'FFFF'0000'0000 >> 32; }
			//constexpr static Id getInputPart(Id id) noexcept { return id & 0x0000'0000'FFFF'0000 >> 16; }
			//constexpr static Id getOutputPart(Id id) noexcept { return id & 0x0000'0000'0000'FFFF; }

			struct Node {
				std::vector<Id> inputs;
				std::vector<Id> outputs;
				std::vector<Id> links;
			};

			struct Link {
				Id fromPinId;
				Id toPinId;
			};

			Engine::FlatHashMap<Id, Link> links;
			Engine::FlatHashMap<Id, Node> nodes;

		public:
			MapTestUI();
			~MapTestUI();
			void generate();
			void render();
	};
}
