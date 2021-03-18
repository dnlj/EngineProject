// Engine
#include <Engine/ConfigParser.hpp>
#include <Engine/Base16.hpp>
#include <Engine/Noise/WorleyNoise.hpp>

// Game
#include <Game/MapTestUI.hpp>


namespace ImNode = ax::NodeEditor;

namespace {
	void setStyle() {
		//ImNode::PushStyleColor(ImNode::StyleColor_NodeBg, ImColor(127, 0, 0, 255));
		//ImNode::PushStyleVar(ImNode::StyleVar_NodeRounding, 1.0f);
	}
}

namespace Game {
	using Id = MapTestUI::Id;
	using PinValue = MapTestUI::PinValue;
	using PinType = MapTestUI::PinType;

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

	struct NodeFinal : MapTestUI::Node {
		virtual void render(Id id) override {
			ImNode::BeginNode(id);

			id.input.pin = 1;
			ImNode::BeginPin(id, ImNode::PinKind::Input);
				ImGui::Text("> In");
			ImNode::EndPin();
			ImNode::EndNode();
		}
	};

	struct NodeDisplay : MapTestUI::Node {
		virtual bool getOutputPinValue(Id pin, PinValue& val) override {
			pin.rotate(1);
			return getInputPinValue(pin, val);
		};

		virtual void render(Id id) override {
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

	struct NodeConstant : MapTestUI::Node {
		PinValue value;

		virtual bool getOutputPinValue(Id pin, PinValue& val) override {
			val = value;
			return val.type != PinType::Invalid;
		}
		
		virtual void render(Id id) override {
			ImNode::BeginNode(id);

			// Combo boxes don't work in nodes.
			if (ImGui::Button(MapTestUI::pinTypeToString(value.type))) {
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
					if (ImGui::Button(MapTestUI::pinTypeToString(i))) {
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
		
		virtual void toConfig(Engine::ConfigParser& cfg, const std::string& pre) override {
			cfg.insert(pre + ".ptype", static_cast<int>(value.type));
			cfg.insert(pre + ".data", Engine::Base16::encode(value.data(), value.size()));
		}

		virtual void fromConfig(Engine::ConfigParser& cfg, const std::string& pre) override {
			const auto* data = cfg.get<std::string>(pre + ".data");
			const auto* ptype = cfg.get<int>(pre + ".ptype");
			if (!data || !ptype) {
				ENGINE_WARN("Unable to read node data");
				return;
			}

			value.type = static_cast<PinType>(*ptype);
			Engine::Base16::decode(*data, value.data());
		}
	};

	template<auto Name, class Op>
	struct NodeBinOp : MapTestUI::Node {
		virtual bool getOutputPinValue(Id pin, PinValue& val) override {
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

		virtual void render(Id id) override {
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

	
	#define PASTE_NODE_BIN_OP(N, O)\
		constexpr static const char _name_for_node_##N[] = #N;\
		using Node##N = NodeBinOp<_name_for_node_##N, O>;
	PASTE_NODE_BIN_OP(Add, decltype([](const auto& a, const auto& b, auto& c) -> bool { c = a + b; return true; }));
	PASTE_NODE_BIN_OP(Sub, decltype([](const auto& a, const auto& b, auto& c) -> bool { c = a - b; return true; }));
	PASTE_NODE_BIN_OP(Mul, decltype([](const auto& a, const auto& b, auto& c) -> bool { c = a * b; return true; }));
	PASTE_NODE_BIN_OP(Div, decltype([](const auto& a, const auto& b, auto& c) -> bool { return (b != decltype(b){}) && (c = a / b, true); }));
	#undef PASTE_NODE_BIN_OP

	
	struct NodeNoise : MapTestUI::Node {
		private:
			Id imgId;
			Engine::Noise::WorleyNoise noise;

		public:
			NodeNoise() : noise{42} {
			}
			~NodeNoise() {
				if (imgId.full) { ctx->images.erase(imgId); }
			}

			virtual bool getOutputPinValue(Id pin, PinValue& val) override {
				if (!imgId.full) {
					imgId = pin;

					auto& img = ctx->images[imgId];
					img = ctx->img;
					const auto& fmt = Engine::getPixelFormatInfo(img.format());
					auto& sz = img.size();
					for (int y = 0; y < sz.y; ++y) {
						for (int x = 0; x < sz.x; ++x) {
							const auto i = (y * sz.x + x) * fmt.channels;
							const float32 s = 0.01f;
							const float32 xs = s * static_cast<float32>(x);
							const float32 ys = s * static_cast<float32>(y);
							const byte v = static_cast<byte>(noise.valueF2F1(xs, ys).value * 255.0f);
							img.data()[i + 0] = v;
							img.data()[i + 1] = v;
							img.data()[i + 2] = v;
						}
					}
				}

				val.type = PinType::Image;
				val.asImageId = imgId;
				return true;
			};

			virtual void render(Id id) override {
				ImNode::BeginNode(id);
				ImGui::Text("Worley Noise");
				// TODO: input pin seed
				ImGui::SameLine();

				id.rotate(1);
				ImNode::BeginPin(id, ImNode::PinKind::Output);
					ImGui::Text("Out >");
				ImNode::EndPin();
				ImNode::EndNode();
			}
	};
}

namespace Game {
	MapTestUI::MapTestUI() {
		ImNode::Config cfg = {};
		ctx = ImNode::CreateEditor(&cfg);
		result = ++lastNodeId.node;
		addNode(NodeType::Final, result);

		
		Engine::ConfigParser save;
		save.loadAndTokenize("node_test.dat");

		if (auto* val = save.get<uint64>("Settings.last_node_id"); val) {
			lastNodeId = *val;
		} else {
			ENGINE_WARN("Unable to get last node id");
		}

		std::vector<std::string> prefixes;
		if (auto* val = save.get<std::string>("Settings.node_id_list"); val) {
			auto curr = val->data();
			auto last = curr + val->size();
			Id id = {};

			while (curr < last) {
				auto start = curr;
				while (curr < last && *curr != ',') { ++curr; }
				prefixes.emplace_back(start, curr);
				++curr;
			}
		} else {
			ENGINE_WARN("Unable to get node id list");
		}

		
		std::vector<std::string> linkIds;
		if (auto* val = save.get<std::string>("Settings.link_id_list"); val) {
			auto curr = val->data();
			auto last = curr + val->size();
			Id id = {};

			while (curr < last) {
				auto start = curr;
				while (curr < last && *curr != ',') { ++curr; }
				linkIds.emplace_back(start, curr);
				++curr;
			}
		} else {
			ENGINE_WARN("Unable to get link id list");
		}

		ImNode::SetCurrentEditor(ctx);
		for (const auto& pre : prefixes) {
			const auto* y = save.get<float64>(pre + ".y");
			const auto* x = save.get<float64>(pre + ".x");
			const auto* type = save.get<int>(pre + ".type");
			const auto* id = save.get<uint64>(pre + ".id");

			if (!y || !x || !type || !id) {
				ENGINE_WARN("Unable to build node. Skipping.");
				continue;
			}

			auto& node = addNode(static_cast<NodeType>(*type), *id);
			ImNode::SetNodePosition(*id, {static_cast<float32>(*x), static_cast<float32>(*y)});
			node->fromConfig(save, pre);
		}
		for (const auto& id : linkIds) {
			const auto* out = save.get<uint64>("Links." + id);
			Id in;
			if (!out || (std::from_chars(id.data(), id.data() + id.size(), in.full).ec != std::errc{})) {
				ENGINE_WARN("Unable to build link. Skipping.");
				continue;
			}
			links[in] = *out;
		}
		ImNode::SetCurrentEditor(nullptr);

		{ // Init Texture
			img = {Engine::PixelFormat::RGB8, {512, 512}};
			texture.setStorage(Engine::TextureFormat::RGB8, img.size());
			texture.setFilter(Engine::TextureFilter::NEAREST);
			buildTexture();
		}
	};

	MapTestUI::~MapTestUI() {
		ImNode::SetCurrentEditor(ctx);
		ENGINE_LOG("Saving nodes...");
		Engine::ConfigParser cfg;

		{ // Save nodes
			std::string nodeList = "";
			nodeList.reserve(nodes.size() * 4); // Assume we usually have id < 999 plus comma for each node

			std::string pre;
			for (auto& [id, node] : nodes) {
				pre = "N" + std::to_string(id.full);
				nodeList += pre;
				nodeList += ",";

				const auto pos = ImNode::GetNodePosition(id);
				node->toConfig(cfg, pre);
				cfg.insert(pre + ".y", pos.y);
				cfg.insert(pre + ".x", pos.x);
				cfg.insert(pre + ".type", static_cast<int>(node->type));
				cfg.insert(pre + ".id", id.full);
			}

			if (nodeList.size()) { nodeList.pop_back(); }
			cfg.insert("Settings.node_id_list", nodeList);
			cfg.insert("Settings.last_node_id", lastNodeId.full);
		}

		{ // Save links
			std::string linkList = "";
			std::string instr;
			for (auto& [in, out] : links) {
				instr = std::to_string(in.full);
				cfg.insert("Links." + instr, out.full);
				linkList += instr;
				linkList += ",";
			}
			if (linkList.size()) { linkList.pop_back(); };
			cfg.insert("Settings.link_id_list", linkList);
		}

		cfg.save("node_test.dat");
		ENGINE_LOG("Done saving nodes.");

		ImNode::SetCurrentEditor(nullptr);
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
					addNode(NodeType::Display);
				}
				if (ImGui::Button("Constant")) {
					addNode(NodeType::Constant);
				}
				if (ImGui::Button("Gradient")) {
				}
				if (ImGui::Button("Worley Noise")) {
					addNode(NodeType::WorleyNoise);
				}
				if (ImGui::Button("Open Simplex Noise")) {
				}
				if (ImGui::Button("White Noise")) {
				}

				ImGui::Separator();

				if (ImGui::Button("Add")) {
					addNode(NodeType::Add);
				}
				if (ImGui::Button("Sub")) {
					addNode(NodeType::Sub);
				}
				if (ImGui::Button("Mul")) {
					addNode(NodeType::Mul);
				}
				if (ImGui::Button("Div")) {
					addNode(NodeType::Div);
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

		if (ImGui::Begin("Display")) {
			if (ImGui::Button("Refresh")) {
				buildTexture();
			}
			ImGui::Text("This is the display window");
			// TODO: zoom / pan
			const ImTextureID tid = reinterpret_cast<void*>(static_cast<uintptr_t>(texture.get()));
			ImGui::Image(tid, ImVec2(static_cast<float32>(img.size().x), static_cast<float32>(img.size().y)));
		}
		ImGui::End();
	}

	void MapTestUI::buildTexture() {
		const auto& sz = img.size();
		const auto& fmt = Engine::getPixelFormatInfo(img.format());
		const auto& node = nodes[result];

		Id pin = result;
		pin.input.pin = 1;

		PinValue val = {};

		/*for (int y = 0; y < sz.y; ++y) {
			for (int x = 0; x < sz.x; ++x) {
				if (!node->getInputPinValue(pin, val)) { continue; }
				if (val.type != PinType::Image) { continue; }
				//if (val.type != PinType::Float32) { continue; }
				//
				//const auto i = (y * sz.x + x) * fmt.channels;
				//img.data()[i + 0] = static_cast<byte>(val.asFloat32 * 255);
				//img.data()[i + 1] = 0;
				//img.data()[i + 2] = 0;
			}
		}*/

		if (!node->getInputPinValue(pin, val)) {
			ENGINE_WARN("Unable to get pin output.");
			return;
		}
		if (val.type != PinType::Image) {
			ENGINE_WARN("Output is not an image.");
			return;
		}

		texture.setImage(images[val.asImageId]);
	}
	auto MapTestUI::addNode(NodeType type, Id id) -> NodePtr& {
		if (!id.full) {
			id = ++lastNodeId.node;
		}
		auto& node = nodes[id];

		#define CASE(E, T) \
			case NodeType:: E: { \
				node = std::make_unique<T>(); \
				break; \
			}

		switch(type) {
			CASE(None, Node);
			CASE(Final, NodeFinal);
			CASE(Display, NodeDisplay);
			CASE(Constant, NodeConstant);
			CASE(Add, NodeAdd);
			CASE(Sub, NodeSub);
			CASE(Mul, NodeMul);
			CASE(Div, NodeDiv);
			CASE(WorleyNoise, NodeNoise);
			default: { ENGINE_ERROR("Unknown node type ", static_cast<int>(type)); }
		}

		#undef CASE

		node->ctx = this;
		node->type = type;
		return node;
	}
}
