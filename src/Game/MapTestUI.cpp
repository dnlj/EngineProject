#include <Game/MapTestUI.hpp>

namespace ImNode = ax::NodeEditor;

namespace {
	void drawNode(int& id, const char* title) {
		ImNode::BeginNode(++id);
			ImGui::Text(title);
			ImNode::BeginPin(++id, ImNode::PinKind::Input);
				ImGui::Text("Input 1");
			ImNode::EndPin();

			ImGui::SameLine();

			ImNode::BeginPin(++id, ImNode::PinKind::Output);
				ImGui::Text("Output 1");
			ImNode::EndPin();
		ImNode::EndNode();
	}

	void setStyle() {
		ImNode::PushStyleColor(ImNode::StyleColor_NodeBg, ImColor(127, 0, 0, 255));
	}
}

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
		setStyle();
		ImNode::Begin("Test Editor");

		// TODO: add nodes: BeginPopupContextWindow

		int id = 0;
		drawNode(id, "First node");
		drawNode(id, "Second node");

		if (ImNode::BeginCreate()) {
			ImNode::PinId pinFromId;
			if (ImNode::QueryNewNode(&pinFromId)) {
				if (ImNode::AcceptNewItem()) {
					ENGINE_LOG("Create Node! ", pinFromId.Get());
					// TODO: impl
				}
			}

			ImNode::PinId pinToId;
			if (ImNode::QueryNewLink(&pinFromId, &pinToId)) {
				if (ImNode::AcceptNewItem()) {
					ENGINE_LOG("New Link! ", pinFromId.Get(), " --> ", pinToId.Get());

					links.emplace(
						static_cast<Id>(rand()), // TODO: dont rand
						Link{pinFromId.Get(), pinToId.Get()}
					);
				}
			}
		}
		ImNode::EndCreate();

		if (ImNode::BeginDelete()) {
			ImNode::NodeId nodeId;
			while (ImNode::QueryDeletedNode(&nodeId)) {
				ENGINE_LOG("Delete node!");
				// TODO: impl
			}

			ImNode::LinkId linkId;
			while (ImNode::QueryDeletedLink(&linkId)) {
				ENGINE_LOG("Delete Link! ", linkId.Get());
				if (ImNode::AcceptDeletedItem()) {
					links.erase(linkId.Get());
				}
			}
		}
		ImNode::EndDelete();

		// Draw links
		for (auto& [id, link] : links) {
			ImNode::Link(id, link.fromPinId, link.toPinId);
		}

		ImNode::End();
		ImNode::SetCurrentEditor(nullptr);
		ImGui::End();
	}
}
