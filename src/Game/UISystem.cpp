// STD
#include <regex>
#include <algorithm>

// Engine
#include <Engine/ImGui/ImGui.hpp>

// Game
#include <Game/UISystem.hpp>
#include <Game/World.hpp>

namespace {
	const double avgDeltaTime = 1/64.0;

	bool connectTo(const std::string& uri, Engine::EngineInstance& engine, Game::World& world) {
		// TODO: use Engine::Net::hostToAddress
		addrinfo* results = nullptr;
		addrinfo hints = {
			.ai_family = AF_INET, // TODO: support ipv6 - AF_UNSPEC
			.ai_socktype = SOCK_DGRAM,
		};

		std::regex regex{R"(^(?:(.*):\/\/)?(.*?)(?::(\d+))?(?:\/.*)?$)"};
		std::smatch matches;
		std::regex_match(uri, matches, regex);
		ENGINE_ASSERT_WARN(matches.size() == 4, "Incorrect number of captures");

		std::string host = matches[2].str();
		std::string serv = matches[3].matched ? matches[3].str() : matches[1].str();

		if (auto err = getaddrinfo(host.data(), serv.data(), &hints, &results); err) {
			ENGINE_WARN("Address error");
			// TODO: error log message + popup/notification
			return false;
		} else {
			for (auto ptr = results; ptr; ptr = results->ai_next) {
				if (ptr->ai_family != AF_INET) {
					ENGINE_ERROR("Unsuported network family");
				}

				Engine::Net::IPv4Address addr{*ptr->ai_addr};
				std::cout << "Address: " << addr << "\n";
				world.getSystem<Game::NetworkingSystem>().connectTo(addr);
			}
		}

		freeaddrinfo(results);
		return true;
	}
}

namespace Game {
	UISystem::UISystem(SystemArg arg)
		: System{arg}
		, connFilter{world.getFilterFor<Game::ConnectionComponent>()} {
	}

	void UISystem::setup() {
		// TODO: handle errors Engine::Input errors better than just spamming console.
		targetIds = world.getSystem<Game::ActionSystem>().getId("Target_X", "Target_Y");
	}

	void UISystem::run(float32 dt) {
		now = Engine::Clock::now();
		rollingWindow = now - rollingWindowSize;
		update = now - lastUpdate >= updateRate;

		if (update) {
			lastUpdate = now;
		}

		frameData.emplace(FrameData{
			.dt = dt,
		}, now);

		// Cull old data
		while(!frameData.empty() && frameData.back().second < rollingWindow) {
			frameData.pop();
		}

		Engine::ImGui::newFrame();
		ui_connect();
		ui_debug();
		Engine::ImGui::draw();
	}

	void UISystem::tick(float32 dt) {
	}

	void UISystem::ui_debug() {
		if (!ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_MenuBar)) { ImGui::End(); return; }

		if (update) {
			fps = 0.0f;
			int32 count = 0;
			auto min = now - fpsAvgWindow;
			for (auto& [fd, time] : frameData) {
				if (time < min) { continue; } else { ++count; }
				fps += fd.dt;
			}
			fps = count / fps;
		}

		ImGui::Text("Avg FPS %f (%f)", fps, 1.0f / fps);

		if (ImGui::Button("Disconnect")) {
			std::vector<Engine::ECS::Entity> ents = {connFilter.cbegin(), connFilter.cend()};
			for (const auto ent : ents) {
				world.getSystem<Game::NetworkingSystem>().disconnect(ent);
			}
		}

		ui_coordinates();
		ui_network();

		ImGui::End();
	}

	void UISystem::ui_connect() {
		auto& netSys = world.getSystem<Game::NetworkingSystem>();
		if (netSys.connectionsCount()) { return; }

		auto& io = ImGui::GetIO();
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::SetNextWindowPos(0.5f * io.DisplaySize, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
		ImGui::Begin("Join Server", nullptr, flags);

		static char serverText[64] = "localhost:21212";
		static Engine::Clock::TimePoint nextRefresh;

		const auto now = Engine::Clock::now();
		if (nextRefresh <= now) {
			nextRefresh = now + std::chrono::seconds{2};
			world.getSystem<Game::NetworkingSystem>().broadcastDiscover();
		}

		ImGui::PushItemWidth(io.DisplaySize.x * 0.5f);

		// TODO: imgui tables https://github.com/ocornut/imgui/issues/2957
		ImGui::Columns(4);
		ImGui::Text("Name");
		ImGui::NextColumn();
		ImGui::Text("Players");
		ImGui::NextColumn();
		ImGui::Text("Ping");
		ImGui::NextColumn();
		ImGui::Text("");
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::Columns(4);

		#if ENGINE_CLIENT
		for (const auto& [addr, info] : netSys.servers) {
			int c = 0;
			ImGui::Text(info.name.c_str());

			ImGui::NextColumn();
			ImGui::Text("2/8");

			ImGui::NextColumn();
			ImGui::Text("100 ms");

			ImGui::NextColumn();
			if (ImGui::Button("Connect")) { // TODO: do i need to PushId ?
				netSys.connectTo(addr);
			}
			ImGui::NextColumn();
		}
		#endif

		ImGui::Columns(1);

		bool shouldConnect = false;
		shouldConnect |= ImGui::InputText("", serverText, sizeof(serverText), ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		shouldConnect |= ImGui::Button("Connect##IP");

		if (shouldConnect && strlen(serverText)) {
			connectTo(serverText, engine, world);
		};

		ImGui::End();
	}

	void UISystem::ui_coordinates() {
		if (!ImGui::CollapsingHeader("Coordinates", ImGuiTreeNodeFlags_DefaultOpen)) { return; }
		auto& mapSys = world.getSystem<Game::MapSystem>();

		auto& actC = world.getComponent<Game::ActionComponent>(Engine::ECS::Entity{74, 1}); // TODO: dont hardcode
		auto screenMousePos = actC.getValue<float32>(targetIds);
		ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);

		auto worldMousePos = engine.camera.screenToWorld(screenMousePos);
		ImGui::Text("Mouse (world): (%f, %f)", worldMousePos.x, worldMousePos.y);
			
		auto blockMousePos = mapSys.worldToBlock(worldMousePos);
		ImGui::Text("Mouse (block): (%i, %i)", blockMousePos.x, blockMousePos.y);

		auto blockWorldMousePos = mapSys.blockToWorld(blockMousePos);
		ImGui::Text("Mouse (block-world): (%f, %f)", blockWorldMousePos.x, blockWorldMousePos.y);

		auto chunkMousePos = mapSys.blockToChunk(blockMousePos);
		auto chunkBlockMousePos = mapSys.chunkToBlock(chunkMousePos);
		ImGui::Text("Mouse (chunk): (%i, %i) (%i, %i)", chunkMousePos.x, chunkMousePos.y, chunkBlockMousePos.x, chunkBlockMousePos.y);

		const auto regionMousePos = mapSys.chunkToRegion(chunkMousePos);
		ImGui::Text("Mouse (region): (%i, %i)", regionMousePos.x, regionMousePos.y);
			
		auto camPos = engine.camera.getPosition();
		ImGui::Text("Camera: (%f, %f, %f)", camPos.x, camPos.y, camPos.z);

		auto mapOffset = world.getSystem<Game::PhysicsOriginShiftSystem>().getOffset();
		ImGui::Text("Map Offset: (%i, %i)", mapOffset.x, mapOffset.y);

		auto mapBlockOffset = mapSys.getBlockOffset();
		ImGui::Text("Map Offset (block): (%i, %i)", mapBlockOffset.x, mapBlockOffset.y);

		auto mapChunkOffset = mapSys.blockToChunk(mapBlockOffset);
		ImGui::Text("Map Offset (chunk): (%i, %i)", mapChunkOffset.x, mapChunkOffset.y);


		#if defined(DEBUG_PHYSICS)
			auto& physDebug = world.getSystem<Game::PhysicsSystem>().getDebugDraw();
			ImGui::Text("Physics Debug Verts: (%i)", physDebug.getVertexCount());
		#endif
	}

	void UISystem::ui_network() {
		if (!ImGui::CollapsingHeader("Networking", ImGuiTreeNodeFlags_DefaultOpen)) { return; }
		auto& [fd, now] = frameData.front();

		for (const auto ent : connFilter) {
			const auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;

			const auto& dt = Engine::Clock::Seconds{now - conn.connectTime}.count();
			fd.sent = conn.writer.totalBytesWritten();
			fd.recv = conn.reader.totalBytesRead();
			ImGui::Text("Sent: %i %.1fb/s     Recv: %i %.1fb/s", fd.sent, fd.sent / dt, fd.recv, fd.recv / dt);
			const auto end = Engine::Clock::Seconds{now.time_since_epoch()}.count();
			const auto begin = Engine::Clock::Seconds{rollingWindow.time_since_epoch()}.count();

			const auto func = [](void* data, int idx) -> ImVec2 {
				auto& self = *reinterpret_cast<decltype(this)>(data);
				auto curr = self.frameData.begin() + idx;

				if (isnan(curr->first.sentDiff)) {
					const auto count = idx - std::max(0, idx - 10);
					auto last = curr - count;
					auto ydiff = static_cast<float32>(curr->first.sent - last->first.sent);
					auto xdiff = Engine::Clock::Seconds{curr->second - last->second}.count();
					curr->first.sentDiff = ydiff / xdiff;
				}

				return {Engine::Clock::Seconds{curr->second.time_since_epoch()}.count(), curr->first.sentDiff};
			};

			constexpr auto yAxisflags = ImPlotAxisFlags_Auxiliary;
			constexpr auto xAxisflags = yAxisflags & ~ImPlotAxisFlags_TickLabels;
			ImPlot::SetNextPlotLimitsX(begin, end, ImGuiCond_Always);
			// TODO: set default y limits Cond_Once?
			if (ImPlot::BeginPlot("##Netgraph", nullptr, nullptr, ImVec2(-1,200), ImPlotFlags_Default, xAxisflags, yAxisflags)) {
				ImPlot::PlotLine("Avg Bytes / Second", func, this, frameData.size(), 0);
				ImPlot::EndPlot();
			}
		}
	}
}
