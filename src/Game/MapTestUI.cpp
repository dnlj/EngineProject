#include <Game/MapTestUI.hpp>
#include <imgui_node_editor_internal.h> // TODO: rm for debugging

namespace ImNode = ax::NodeEditor;

namespace {
	void drawNode(Game::MapTestUI::Id id, const char* title) {
		ImNode::BeginNode(id);
			ImGui::Text(title);
			id.pin1.pin = 1;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				ImGui::Text("Input 1");
			ImNode::EndPin();
			
			ImGui::SameLine();
			
			id.pin1.pin = 2;
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				ImGui::Text("Output 1");
			ImNode::EndPin();
		ImNode::EndNode();
	}

	void setStyle() {
		//ImNode::PushStyleColor(ImNode::StyleColor_NodeBg, ImColor(127, 0, 0, 255));
		//ImNode::PushStyleVar(ImNode::StyleVar_NodeRounding, 1.0f);
	}
}

namespace Game {
	MapTestUI::MapTestUI() {
		//ImNode::Config cfg; // TODO: file, save/load
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
		setStyle();
		ImNode::Begin("Test Editor");

		// TODO: add nodes: BeginPopupContextWindow

		//int id = 0;
		//drawNode(id, "First node");
		//drawNode(id, "Second node");

		if (ImNode::BeginCreate()) {
			ImNode::PinId pinFromId;
			if (ImNode::QueryNewNode(&pinFromId)) {
				if (ImNode::AcceptNewItem()) {
					ENGINE_LOG("Create Node! ", pinFromId.Get());
					
					ImNode::Suspend();
					ImGui::OpenPopup("New Node");
					ImNode::Resume();
				}
			}

			ImNode::PinId pinToId;
			if (ImNode::QueryNewLink(&pinFromId, &pinToId)) {
				// TODO: that one id is an input and the other is an output
				if (ImNode::AcceptNewItem()) {
					Id link{pinFromId, pinToId};
					ENGINE_LOG("New Link! ", link.string());
					links.emplace(link, Link{});
				}
			}
		}
		ImNode::EndCreate();

		if (ImNode::BeginDelete()) {
			ImNode::NodeId nodeId;
			while (ImNode::QueryDeletedNode(&nodeId)) {
				ENGINE_LOG("Delete node!");
				nodes.erase(nodeId);
			}

			ImNode::LinkId linkId;
			while (ImNode::QueryDeletedLink(&linkId)) {
				ENGINE_LOG("Delete Link!");
				if (ImNode::AcceptDeletedItem()) {
					links.erase(linkId);
				}
			}
		}
		ImNode::EndDelete();
		
		// Render nodes
		// Nodes must be rendered BEFORE links
		for (auto& [id, node] : nodes) {
			std::string label = "Node " + std::to_string(id);
			drawNode(id, label.c_str());
		}

		// Render links
		// Links must be rendered AFTER nodes
		for (auto& [id, link] : links) {
			ImNode::Link(id, id.pin1, id.pin2);
		}

		// Context Menus
		ImNode::Suspend();
		{
			if (ImNode::NodeId id; ImNode::ShowNodeContextMenu(&id)) {
				ENGINE_LOG("Node context");
				ImGui::OpenPopup("Test Popup");
			} else if (ImNode::PinId id; ImNode::ShowPinContextMenu(&id)) {
				ENGINE_LOG("Pin context");
				ImGui::OpenPopup("Test Popup");
			} else if (ImNode::LinkId id; ImNode::ShowLinkContextMenu(&id)) {
				ENGINE_LOG("Link context");
				ImGui::OpenPopup("Test Popup");
			} else if (ImNode::ShowBackgroundContextMenu()) {
				ENGINE_LOG("Background context");
				ImGui::OpenPopup("Test Popup");
			}

			if (ImGui::BeginPopup("Test Popup")) {
				ImGui::Text("Test~~~~~~~~~~~~~~");

				if (ImGui::Button("Create Node")) {
					ImGui::CloseCurrentPopup();
					++lastNodeId.node;
					nodes[lastNodeId] = {};
				}

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("New Node")) {
				ImGui::Button("Constant");
				ImGui::Button("Gradient");
				ImGui::Button("Worley Noise");
				ImGui::Button("Open Simplex Noise");
				ImGui::Button("White Noise");
				ImGui::Separator();
				ImGui::Button("Add");
				ImGui::Button("Subtract");
				ImGui::Button("Multiply");
				ImGui::Button("Divide");
				ImGui::Button("Modulus");
				ImGui::Button("DivMod");
				ImGui::Separator();
				ImGui::Button("Min");
				ImGui::Button("Max");
				ImGui::Button("Abs");
				ImGui::Button("Lerp");
				ImGui::EndPopup();
			}
		}
		ImNode::Resume();

		ImNode::End();
		ImNode::SetCurrentEditor(nullptr);
		ImGui::End();
	}
}
