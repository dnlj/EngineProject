#include <Game/MapTestUI.hpp>
#include <imgui_node_editor_internal.h> // TODO: rm for debugging

namespace ImNode = ax::NodeEditor;

namespace {
	void setStyle() {
		//ImNode::PushStyleColor(ImNode::StyleColor_NodeBg, ImColor(127, 0, 0, 255));
		//ImNode::PushStyleVar(ImNode::StyleVar_NodeRounding, 1.0f);
	}

}

namespace Game {
	void MapTestUI::Node::render(MapTestUI::Id id) {
		ImNode::BeginNode(id);
		ImGui::Text("This is a node!");
		int i = 0;
		std::string title;

		while (i < 6) {
			id.pin1.pin = ++i;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				title = "Input " + std::to_string(i);
				ImGui::Text(title.c_str());
			ImNode::EndPin();

			ImGui::SameLine();
			id.pin1.pin = ++i;
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				title = "Output " + std::to_string(i);
				ImGui::Text(title.c_str());
			ImNode::EndPin();
		}

		ImNode::EndNode();
	}

	struct MapTestUI::NodeConstant : MapTestUI::Node {
		PinValue value;
		NodeConstant(float32 val) : value{.type = PinType::Float32, .asFloat32 = val} {};

		virtual bool getOutputPinValue(Id pin, PinValue& val) {
			val = value;
			return true;
		}
		
		virtual void render(Id id) {
			ImNode::BeginNode(id);

			ImGui::SetNextItemWidth(64);
			ImGui::DragFloat("Value", &value.asFloat32);

			ImGui::SameLine();

			id.pin1.pin = 1;
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				ImGui::Text("Out >");
			ImNode::EndPin();

			ImNode::EndNode();
		}
	};

//	struct MapTestUI::NodeAdd : MapTestUI::Node {
//		virtual bool getOutputPinValue(Id pin, PinValue& val) {
//
//		}
//	};
}

namespace Game {
	MapTestUI::MapTestUI() {
		//ImNode::Config cfg; // TODO: file, save/load
		ctx = ImNode::CreateEditor(nullptr);
		result = ++lastNodeId.node;
		nodes[result] = std::make_unique<Node>();
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

		if (ImNode::BeginCreate()) {
			ImNode::PinId pin1;
			if (ImNode::QueryNewNode(&pin1)) {
				if (ImNode::AcceptNewItem()) {
					ENGINE_LOG("Create Node! ", pin1.Get());
					
					ImNode::Suspend();
					ImGui::OpenPopup("New Node");
					ImNode::Resume();
				}
			}

			ImNode::PinId pin2;
			if (ImNode::QueryNewLink(&pin1, &pin2)) {
				//
				// TODO: that one id is an input and the other is an output. sort correctly
				if (ImNode::AcceptNewItem()) {
					Id link{pin2, pin1};
					ENGINE_LOG("New Link! ", link.string());
					links[pin2] = pin1;
				}
			}
		}
		ImNode::EndCreate();

		if (ImNode::BeginDelete()) {
			ImNode::NodeId nodeId;
			while (ImNode::QueryDeletedNode(&nodeId)) {
				if (result == Id{nodeId}) {
					ENGINE_LOG("DELETE REJECT");
					ImNode::RejectDeletedItem();
					continue;
				}
				nodes.erase(nodeId);
			}

			ImNode::LinkId linkId;
			while (ImNode::QueryDeletedLink(&linkId)) {
				if (ImNode::AcceptDeletedItem()) {
					links.erase(Id{linkId}.pin1);
				}
			}
		}
		ImNode::EndDelete();
		
		// Render nodes
		// Nodes must be rendered BEFORE links
		for (auto& [id, node] : nodes) {
			ImGui::PushID(id.node);
			node->render(id);
			ImGui::PopID();
		}

		// Render links
		// Links must be rendered AFTER nodes
		for (auto& [in, out] : links) {
			ImNode::Link(Id{in, out}, out, in);
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
				ImGui::OpenPopup("New Node");
			}

			if (ImGui::BeginPopup("Test Popup")) {
				ImGui::Text("Test~~~~~~~~~~~~~~");

				if (ImGui::Button("Create Node")) {
					ImGui::CloseCurrentPopup();
					++lastNodeId.node;
					nodes[lastNodeId] = std::make_unique<Node>();
				}

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("New Node")) {
				if (ImGui::Button("Constant")) {
					++lastNodeId.node;
					nodes[lastNodeId] = std::make_unique<NodeConstant>(1.0f);
				}
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
