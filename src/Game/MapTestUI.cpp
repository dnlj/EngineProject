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
			id.input.pin = ++i;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				title = "Input " + std::to_string(i);
				ImGui::Text(title.c_str());
			ImNode::EndPin();

			ImGui::SameLine();
			id.rotate(++i);
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				title = "Output " + std::to_string(i);
				ImGui::Text(title.c_str());
			ImNode::EndPin();
			id.rotate();
		}

		ImNode::EndNode();
	}
	
	struct MapTestUI::NodeDisplay : MapTestUI::Node {
		virtual bool getOutputPinValue(Id pin, PinValue& val) {
			pin.rotate(1);
			return getInputPinValue(pin, val);
		};

		virtual void render(Id id) {
			ImNode::BeginNode(id);

			id.input.pin = 1;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				ImGui::Text("> In");
			ImNode::EndPin();

			ImGui::SameLine();

			if (PinValue val; getInputPinValue(id, val)) {
				switch (val.type) {
					case PinType::Bool: { ImGui::Text("%i", val.asBool); break; }
					case PinType::Int32: { ImGui::Text("%i", val.asInt32); break; }
					case PinType::Float32: { ImGui::Text("%1.3f", val.asFloat32); break; }
					case PinType::Vec2: { ImGui::Text("{ %1.3f, %1.3f }", val.asVec2.x, val.asVec2.y); break; }
					case PinType::Vec3: { ImGui::Text("{ %1.3f, %1.3f, %1.3f }", val.asVec3.x, val.asVec3.y, val.asVec3.z); break; }
					case PinType::Vec4: { ImGui::Text("{ %1.3f, %1.3f, %1.3f, %1.3f }", val.asVec4.x, val.asVec4.y, val.asVec4.z, val.asVec4.w); break; }
					default: { ImGui::Text("Invalid input"); }
				}
			} else {
				ImGui::Text("No Input");
			}

			ImGui::SameLine();

			id.rotate(2);
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				ImGui::Text("Out >");
			ImNode::EndPin();
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

			// Combo boxes don't work in nodes.
			if (ImGui::Button(pinTypeToString(value.type))) {
				ImGui::OpenPopup("SelectType");
			}

			ImGui::SetNextItemWidth(150);
			switch (value.type) {
				case PinType::Bool: { ImGui::Checkbox("Value", &value.asBool); break; }
				case PinType::Int32: { ImGui::DragInt("Value", &value.asInt32); break; }
				case PinType::Float32: { ImGui::DragFloat("Value", &value.asFloat32); break; }
				case PinType::Vec2: { ImGui::DragFloat2("Value", &value.asVec2[0]); break; }
				case PinType::Vec3: { ImGui::DragFloat3("Value", &value.asVec3[0]); break; }
				case PinType::Vec4: { ImGui::DragFloat4("Value", &value.asVec4[0]); break; }
				default: { ImGui::Text("Invalid input"); }
			}

			ImGui::SameLine();

			id.rotate(1);
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				ImGui::Text("Out >");
			ImNode::EndPin();

			ImNode::EndNode();

			ImNode::Suspend();
			if (ImGui::BeginPopup("SelectType")) {
				for (PinType i = {}; i < PinType::_COUNT; ++i) {
					if (ImGui::Button(pinTypeToString(i))) {
						ENGINE_LOG("Selected! ", (int)i);
						value.zero();
						value.type = i;
						ImGui::CloseCurrentPopup();
						break;
					}
				}
				ImGui::EndPopup();
			}
			ImNode::Resume();
		}
	};

	template<auto Name, class Op>
	struct MapTestUI::NodeBinOp : MapTestUI::Node {
		virtual bool getOutputPinValue(Id pin, PinValue& val) {
			pin.rotate(1);
			PinValue a;
			if (!getInputPinValue(pin, a)) { return false; }

			pin.input.pin = 2;
			PinValue b;
			if (!getInputPinValue(pin, b)) { return false; }

			if (a.type != b.type) { return false; }

			// TODO: need to handle type combos. i want to be able to write vec4*int and similar
			val.type = a.type;
			switch (a.type) {
				//case PinType::Bool: { return Op{}(a.asBool, b.asBool, val.asBool); }
				case PinType::Int32: { return Op{}(a.asInt32, b.asInt32, val.asInt32); }
				case PinType::Float32: { return Op{}(a.asFloat32, b.asFloat32, val.asFloat32); }
				case PinType::Vec2: { return Op{}(a.asVec2, b.asVec2, val.asVec2); }
				case PinType::Vec3: { return Op{}(a.asVec3, b.asVec3, val.asVec3); }
				case PinType::Vec4: { return Op{}(a.asVec4, b.asVec4, val.asVec4); }
			}

			val.type = PinType::Invalid;
			return false;
		}

		virtual void render(Id id) {
			ImNode::BeginNode(id);

			ImGui::Text(Name);

			id.input.pin = 1;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				ImGui::Text("> A");
			ImNode::EndPin();

			ImGui::SameLine();

			id.rotate(3);
			ImNode::BeginPin(id, ImNode::PinKind::Output);
				ImGui::Text("Result >");
			ImNode::EndPin();
			
			id.rotate(2);
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				ImGui::Text("> B");
			ImNode::EndPin();

			ImNode::EndNode();
		}
	};
}

namespace Game {
	MapTestUI::MapTestUI() {
		ImNode::Config cfg = {};
		//cfg.SettingsFile = nullptr;
		ctx = ImNode::CreateEditor(&cfg);
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
				if (ImNode::AcceptNewItem()) {
					Id out = pin1;
					Id in = pin2;
					if (out.input && in.input || out.output && in.output) {
						ImNode::RejectNewItem();
					} else {
						if (out.input) { std::swap(out, in); }
						ENGINE_LOG("New Link! ", Id{in, out}.string());
						links[in] = out;
					}
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
					links.erase(Id{linkId}.input);
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

				#define PASTE_NODE_BIN_OP(N, O)\
				if (ImGui::Button(N)) {\
					++lastNodeId.node;\
					auto& node = nodes[lastNodeId];\
					constexpr static const char Name[] = N;\
					node = std::make_unique<NodeBinOp<Name, O>>();\
					node->ctx = this;\
				}

				PASTE_NODE_BIN_OP("Add", decltype([](const auto& a, const auto& b, auto& c) -> bool { c = a + b; return true; }));
				PASTE_NODE_BIN_OP("Sub", decltype([](const auto& a, const auto& b, auto& c) -> bool { c = a - b; return true; }));
				PASTE_NODE_BIN_OP("Mul", decltype([](const auto& a, const auto& b, auto& c) -> bool { c = a * b; return true; }));
				PASTE_NODE_BIN_OP("Div", decltype([](const auto& a, const auto& b, auto& c) -> bool { return (b != decltype(b){}) && (c = a / b, true); }));
				#undef PASTE_NODE_BIN_OP

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
