#include <Game/MapTestUI.hpp>

namespace ImNode = ax::NodeEditor;

namespace Game {
	MapTestUI::MapTestUI() {
		ctx = ImNode::CreateEditor(nullptr);
	};

	MapTestUI::~MapTestUI() {
		ImNode::DestroyEditor(ctx);
	}

	void MapTestUI::generate() {
	}

	void MapTestUI::render() {
		if (!ImGui::Begin("Map Playground", nullptr, ImGuiWindowFlags_None)) { ImGui::End(); return; }
		ImGui::Text("Test!");
		ImNode::SetCurrentEditor(ctx);
		ImNode::Begin("Test Editor");

		int id = 0;

		ImNode::BeginNode(id++);
			ImGui::Text("First Node");
			ImNode::BeginPin(id++, ImNode::PinKind::Input);
				ImGui::Text("Input 1");
			ImNode::EndPin();

			ImGui::SameLine();

			ImNode::BeginPin(id++, ImNode::PinKind::Output);
				ImGui::Text("Output 1");
			ImNode::EndPin();
		ImNode::EndNode();

		ImNode::BeginNode(id++);
			ImGui::Text("Second Node");
			ImNode::BeginPin(id++, ImNode::PinKind::Input);
				ImGui::Text("Input 2");
			ImNode::EndPin();

			ImGui::SameLine();

			ImNode::BeginPin(id++, ImNode::PinKind::Output);
				ImGui::Text("Output 2");
			ImNode::EndPin();
		ImNode::EndNode();

		ImNode::End();
		ImGui::End();
	}
}
