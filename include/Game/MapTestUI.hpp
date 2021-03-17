#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ImGui/ImGui.hpp>
#include <Engine/FlatHashMap.hpp>
#include <imgui_node_editor.h>
#include <Engine/ConfigParser.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class MapTestUI {
		public:
			/**
			 * Bit layout of ids.
			 * Ids are divided into two 32 bit node-pin pairs.
			 * If both pairs are present then this is a link id.
			 * If only the lower pair is present then this is a input pin id.
			 * If only the high pair is present then this is a output pin id.
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
					PinId input;
					PinId output;
				};

				Id() : full{} {}
				Id(uint64 id) : full{id} {}
				Id(PinId in, PinId out = {}) : input{in}, output{out} {}
				Id(Id in, Id out) : input{in.input}, output{out.output} {}
				Id(const ax::NodeEditor::NodeId& id) : full{id.Get()} {}
				Id(const ax::NodeEditor::LinkId& id) : full{id.Get()} {}
				Id(const ax::NodeEditor::PinId& id) : full{id.Get()} {}
				//Id(const ax::NodeEditor::PinId& p1, const ax::NodeEditor::PinId& p2) : input{p1.Get()}, output{p2.Get()} {}

				operator ax::NodeEditor::NodeId() const { return full; }
				operator ax::NodeEditor::LinkId() const { return full; }
				operator ax::NodeEditor::PinId() const { return full; }

				operator uint64() const { return full; }

				void rotate(uint16 pid = 0) {
					if (input) {
						full <<= 32;
						if (pid) { output.pin = pid; }
					} else {
						full >>= 32;
						if (pid) { input.pin = pid; }
					}
				}

				std::string string() const { return "[("+ std::to_string(output.pin) + ", "+ std::to_string(output.node) + "), (" + std::to_string(input.pin) + ", "+ std::to_string(input.node) + ")]"; }

				struct Hash {
					ENGINE_INLINE size_t operator()(const Id& id) const noexcept {
						return static_cast<size_t>(id);
					}
				};
			}; static_assert(sizeof(Id) == 8, "This type is assumed to be tightly packed.");
			
			enum class PinType {
				Invalid = 0,
				Bool,
				Int32,
				Float32,
				Vec2,
				Vec3,
				Vec4,
				_COUNT,
			};

			// TODO: move to xmacro. its the simpleest solution
			constexpr static const char* pinTypeToString(PinType type) noexcept {
				const int i = static_cast<int>(type);
				constexpr const char* names[] = {"Invalid","Bool","Int32","Float32","Vec2","Vec3","Vec4"};
				if (i < 0 || i >= std::size(names)) { return names[0]; }
				return names[i];
			}

			enum class PinDirection {
				Invalid = 0,
				Input,
				Output,
				_COUNT,
			};

			struct PinMeta {
				PinDirection dir;
				PinType type;

			};

			struct PinValue {
				PinType type;
				union {
					byte asByte;
					bool asBool;
					int32 asInt32;
					float32 asFloat32;
					glm::vec2 asVec2;
					glm::vec3 asVec3;
					glm::vec4 asVec4;
				};

				PinValue() : type{} {}
				PinValue(bool val) : type{PinType::Bool}, asBool{val} {}
				PinValue(int32 val) : type{PinType::Int32}, asInt32{val} {}
				PinValue(float32 val) : type{PinType::Float32}, asFloat32{val} {}
				PinValue(glm::vec2 val) : type{PinType::Vec2}, asVec2{val} {}
				PinValue(glm::vec3 val) : type{PinType::Vec3}, asVec3{val} {}
				PinValue(glm::vec4 val) : type{PinType::Vec4}, asVec4{val} {}

				// TODO: toString()
				
				const byte* data() const noexcept { return &asByte; }
				byte* data() noexcept { return &asByte; }
				constexpr static size_t size() noexcept { return sizeof(PinValue) - offsetof(PinValue, asByte); }

				void zero() {
					memset(data(), 0, size());
				}
			};
			
			enum class NodeType : int {
				None,
				Display,
				Constant,
				Add,
				Sub,
				Mul,
				Div,
			};

			struct Node {
				// Input and output pins share the same id sequence
				MapTestUI* ctx = nullptr;
				std::vector<PinMeta> pins;
				NodeType type;
				virtual ~Node() {}

				virtual bool getOutputPinValue(Id pin, PinValue& val) {
					return false;
				};

				virtual bool getInputPinValue(Id pin, PinValue& val) {
					auto link = ctx->links.find(pin);
					if (link == ctx->links.end()) { return false; }

					Id out = link->second;
					auto& node = ctx->nodes[out.output.node];
					return node->getOutputPinValue(out, val);
				}

				virtual void render(Id id);

				virtual void toConfig(Engine::ConfigParser& cfg, const std::string& pre) {}
				virtual void fromConfig(Engine::ConfigParser& cfg, const std::string& pre) {}
			};

		private:
			ax::NodeEditor::EditorContext* ctx;
			Id lastNodeId = 0;
			Id result;

			/** Stores input pin -> output pin pairs. */
			Engine::FlatHashMap<Id, Id, Id::Hash> links;

			using NodePtr = std::unique_ptr<Node>;
			Engine::FlatHashMap<Id, NodePtr, Id::Hash> nodes;

			NodePtr& addNode(NodeType type, Id id = {});

		public:
			MapTestUI();
			~MapTestUI();
			void generate();
			void render();
	};
}

// TODO: move pin type into own file.
ENGINE_BUILD_ALL_OPS(::Game::MapTestUI::PinType);
