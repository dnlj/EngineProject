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
	
	struct MapTestUI::NodeDisplay : MapTestUI::Node {
		virtual void render(Id id) {
			ImNode::BeginNode(id);

			id.pin1.pin = 1;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				ImGui::Text("> In");
			ImNode::EndPin();

			if (PinValue val; getInputPinValue(id, val)) {
				switch (val.type) {
					case PinType::Bool: { ImGui::Text("Input: %i", val.asBool); break; }
					case PinType::Int32: { ImGui::Text("Input: %i", val.asInt32); break; }
					case PinType::Float32: { ImGui::Text("Input: %1.3f", val.asFloat32); break; }
					case PinType::Vec2: { ImGui::Text("Input: <%1.3f, %1.3f>", val.asVec2.x, val.asVec2.y); break; }
					case PinType::Vec3: { ImGui::Text("Input: <%1.3f, %1.3f, %1.3f>", val.asVec3.x, val.asVec3.y, val.asVec3.z); break; }
					case PinType::Vec4: { ImGui::Text("Input: <%1.3f, %1.3f, %1.3f, %1.3f>", val.asVec4.x, val.asVec4.y, val.asVec4.z, val.asVec4.w); break; }
					default: { ImGui::Text("Invalid input"); }
				}
			} else {
				ImGui::Text("No Input");
			}

			ImNode::EndNode();
		}
	};

	struct MapTestUI::NodeConstant : MapTestUI::Node {
		PinValue value;
		NodeConstant(PinValue val) : value{val} {};

		virtual bool getOutputPinValue(Id pin, PinValue& val) {
			val = value;
			return val.type != PinType::Invalid;
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
				if (ImGui::Button("Display")) {
					++lastNodeId.node;
					auto& node = nodes[lastNodeId];
					node = std::make_unique<NodeDisplay>();
					node->ctx = this;
				}
				if (ImGui::Button("Constant")) {
					++lastNodeId.node;
					auto& node = nodes[lastNodeId];
					node = std::make_unique<NodeConstant>(1.0f);
					node->ctx = this;
				}
				if (ImGui::Button("Gradient")) {
				}
				if (ImGui::Button("Worley Noise")) {
				}
				if (ImGui::Button("Open Simplex Noise")) {
				}
				if (ImGui::Button("White Noise")) {
				}

				ImGui::Separator();

				if (ImGui::Button("Add")) {
				}
				if (ImGui::Button("Subtract")) {
				}
				if (ImGui::Button("Multiply")) {
				}
				if (ImGui::Button("Divide")) {
				}
				if (ImGui::Button("Modulus")) {
				}
				if (ImGui::Button("DivMod")) {
				}

				ImGui::Separator();

				if (ImGui::Button("Min")) {
				}
				if (ImGui::Button("Max")) {
				}
				if (ImGui::Button("Abs")) {
				}
				if (ImGui::Button("Lerp")) {
				}
				ImGui::EndPopup();
			}
		}
		ImNode::Resume();

		ImNode::End();
		ImNode::SetCurrentEditor(nullptr);
		ImGui::End();
	}
}
