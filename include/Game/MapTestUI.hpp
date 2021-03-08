#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ImGui/ImGui.hpp>
#include <imgui_node_editor.h>


namespace Game {
	class MapTestUI {
		private:
			ax::NodeEditor::EditorContext* ctx;

		public:
			MapTestUI();
			~MapTestUI();
			void generate();
			void render();
	};
}
