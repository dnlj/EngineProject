// STD
#include <regex>
#include <algorithm>
#include <cstdio>

// Engine
#include <Engine/Glue/Box2D.hpp>
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/systems/UISystem.hpp>
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
				ENGINE_LOG("Address: ", addr);
				world.getSystem<Game::NetworkingSystem>().connectTo(addr);
			}
		}

		freeaddrinfo(results);
		return true;
	}

}

namespace Game {
	UISystem::UISystem(SystemArg arg)
		: System{arg} {
	}

	void UISystem::setup() {
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
		while(!frameData.empty() && frameData.front().second < rollingWindow) {
			frameData.pop();
		}

		if constexpr (ENGINE_CLIENT) {
			//ImGui::ShowDemoWindow();
			//mapTestUI.render();
			ui_connect();
		}

		if constexpr (ENGINE_SERVER) {
		}

		ui_debug();
	}

	void UISystem::tick() {
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
		ImGui::Text("Tick %i", world.getTick());
		ImGui::Text("Tick Scale: %.4f", world.tickScale);

		if (ImGui::Button("Disconnect")) {
			for (const auto& ent : world.getFilter<ConnectionComponent>()) {
				const auto& addr = world.getComponent<ConnectionComponent>(ent).conn->address();
				world.getSystem<NetworkingSystem>().requestDisconnect(addr);
			}
		}

		ui_coordinates();
		ui_render();
		ui_camera();
		ui_netsim();
		ui_nethealth();
		ui_network();
		ui_entities();

		ImGui::End();
	}

	void UISystem::ui_connect() {
		#if ENGINE_CLIENT
		auto& netSys = world.getSystem<Game::NetworkingSystem>();
		if (netSys.playerCount()) { return; }

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

		ImGui::Columns(1);

		bool shouldConnect = false;
		shouldConnect |= ImGui::InputText("", serverText, sizeof(serverText), ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		shouldConnect |= ImGui::Button("Connect##IP");

		if (shouldConnect && strlen(serverText)) {
			connectTo(serverText, engine, world);
		};

		ImGui::End();
		#endif
	}

	void UISystem::ui_coordinates() {
		if (!ImGui::CollapsingHeader("Coordinates")) { return; }

		const auto& activePlayerFilter = world.getFilter<PlayerFlag>();
		if (activePlayerFilter.empty()) { return; }
		const auto ply = *activePlayerFilter.begin();

		auto& mapSys = world.getSystem<Game::MapSystem>();

		const auto& actComp = world.getComponent<Game::ActionComponent>(ply);
		if (!actComp.valid()) { return; }
		// TODO: reimplement - ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);

		const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);

		const auto offsetMousePos = actComp.getTarget();
		ImGui::Text("Mouse (offset): (%f, %f)", offsetMousePos.x, offsetMousePos.y);

		const auto worldMousePos = offsetMousePos + Engine::Glue::as<glm::vec2>(physComp.getPosition());
		ImGui::Text("Mouse (world): (%f, %f)", worldMousePos.x, worldMousePos.y);
			
		const auto blockMousePos = mapSys.worldToBlock(worldMousePos);
		ImGui::Text("Mouse (block): (%i, %i)", blockMousePos.x, blockMousePos.y);

		const auto blockWorldMousePos = mapSys.blockToWorld(blockMousePos);
		ImGui::Text("Mouse (block-world): (%f, %f)", blockWorldMousePos.x, blockWorldMousePos.y);

		const auto chunkMousePos = mapSys.blockToChunk(blockMousePos);
		const auto chunkBlockMousePos = mapSys.chunkToBlock(chunkMousePos);
		ImGui::Text("Mouse (chunk): (%i, %i) (%i, %i)", chunkMousePos.x, chunkMousePos.y, chunkBlockMousePos.x, chunkBlockMousePos.y);

		const auto regionMousePos = mapSys.chunkToRegion(chunkMousePos);
		ImGui::Text("Mouse (region): (%i, %i)", regionMousePos.x, regionMousePos.y);
			
		const auto camPos = engine.camera.getPosition();
		ImGui::Text("Camera: (%f, %f, %f)", camPos.x, camPos.y, camPos.z);

		const auto mapOffset = world.getSystem<Game::PhysicsOriginShiftSystem>().getOffset();
		ImGui::Text("Map Offset: (%i, %i)", mapOffset.x, mapOffset.y);

		const auto mapBlockOffset = mapSys.getBlockOffset();
		ImGui::Text("Map Offset (block): (%i, %i)", mapBlockOffset.x, mapBlockOffset.y);

		const auto mapChunkOffset = mapSys.blockToChunk(mapBlockOffset);
		ImGui::Text("Map Offset (chunk): (%i, %i)", mapChunkOffset.x, mapChunkOffset.y);


		#if defined(DEBUG_PHYSICS)
			auto& physDebug = world.getSystem<Game::PhysicsSystem>().getDebugDraw();
			ImGui::Text("Physics Debug Verts: (%i)", physDebug.getVertexCount());
		#endif
	}
	
	void UISystem::ui_render() {
		if (!ImGui::CollapsingHeader("Render")) { return; }
		const auto& spriteSys = world.getSystem<SpriteSystem>();
		ImGui::Text("Sprites: %i", spriteSys.totalSprites());
		ImGui::Text("Sprite Groups: %i", spriteSys.totalSpriteGroups());

		if (ImGui::TreeNode("Groups")) {
			for (const auto& group : spriteSys.getSpriteGroups()) {
				ImGui::Text("L: %i    T: %i    C: %i", group.layer, group.texture, group.count);
			}
			ImGui::TreePop();
		}
	}

	void UISystem::ui_camera() {
		if constexpr (!ENGINE_SERVER) { return; }
		if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) { return; }
		for (const auto ply : world.getFilter<PlayerFlag>()) {
			if (world.hasComponent<CameraTargetFlag>(ply)) {}
			ImGui::PushID(ply.id);

			ss.str("");
			ss << ply;
			if (ImGui::Button(ss.str().c_str())) {
				for (const auto ply2 : world.getFilter<PlayerFlag>()) {
					if (world.hasComponent<CameraTargetFlag>(ply2)) {
						world.removeComponent<CameraTargetFlag>(ply2);
					}
				}
				world.addComponent<CameraTargetFlag>(ply);
			};

			ImGui::PopID();
		}
	}

	void UISystem::ui_netsim() {
		if (!ImGui::CollapsingHeader("Network Conditions", 0/* | ImGuiTreeNodeFlags_DefaultOpen */)) { return; }
		#ifndef ENGINE_UDP_NETWORK_SIM
			ImGui::Text("%s", "Network simulation disabled.");
		#else
			auto& netSys = world.getSystem<NetworkingSystem>();
			auto& settings = netSys.getSocket().getSimSettings();

			int hpa = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(settings.halfPingAdd).count());
			ImGui::SliderInt("Half Ping Add", &hpa, 0, 500);
			settings.halfPingAdd = std::chrono::milliseconds{hpa};

			ImGui::SliderFloat("Jitter", &settings.jitter, 0.0f, 1.0f);
			ImGui::SliderFloat("Duplicate Chance", &settings.duplicate, 0.0f, 1.0f);
			ImGui::SliderFloat("Loss", &settings.loss, 0.0f, 1.0f);
		#endif
	}

	void UISystem::ui_network() {
		if (!ImGui::CollapsingHeader("Networking")) { return; }
		
		for (auto ent : world.getFilter<ConnectionComponent>()) {
			if (!world.hasComponent<NetworkStatsComponent>(ent)) {
				world.addComponent<NetworkStatsComponent>(ent);
			}
			auto& statsComp = world.getComponent<NetworkStatsComponent>(ent);
			auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;
			auto& buff = statsComp.buffer;

			while (!buff.empty() && buff.front().time < rollingWindow) {
				buff.pop();
			}
			const auto totalBytesSent = conn.getTotalBytesSent();
			const auto totalBytesRecv = conn.getTotalBytesRecv();

			buff.push({
				.time = now,
				.sent = {
					.diff = static_cast<float32>(totalBytesSent - statsComp.lastTotalBytesSent),
					.avg  = conn.getSendBandwidth(),
				},
				.recv = {
					.diff = static_cast<float32>(totalBytesRecv - statsComp.lastTotalBytesRecv),
					.avg  = conn.getRecvBandwidth(),
				},
			});

			statsComp.lastTotalBytesSent = totalBytesSent;
			statsComp.lastTotalBytesRecv = totalBytesRecv;

			const auto& data = buff.back();

			if (update) {
				statsComp.displaySentTotal = totalBytesSent; 
				statsComp.displayRecvTotal = totalBytesRecv;
				statsComp.displaySentAvg = data.sent.avg;
				statsComp.displayRecvAvg = data.recv.avg;

				statsComp.displayPing = Engine::Clock::Seconds{conn.getPing()}.count() * 1000.0f;
				statsComp.displayJitter = Engine::Clock::Seconds{conn.getJitter()}.count() * 1000.0f;
				statsComp.displayLoss = conn.getLoss();

				statsComp.displayInputBufferSize = statsComp.inputBufferSize;
				statsComp.displayIdealInputBufferSize = statsComp.idealInputBufferSize;
			}

			float32 estbuff = 0;
			if (world.hasComponent<ActionComponent>(ent)) {
				estbuff = world.getComponent<ActionComponent>(ent).estBufferSize;
			}

			const auto& addr = conn.address();
			ImGui::Text(
				"%i.%i.%i.%i:%i\n"
				"Ping: %.1fms          Jitter: %.1fms    Est. Buffer: %.2f\n"
				"Buffer Size: %i     Ideal: %.3f\n"
				"Sent: %ib %.1fb/s     Recv: %ib %.1fb/s     Loss: %.3f"
				"\nBudget: %.2f"
				,
				addr.a, addr.b, addr.c, addr.d, addr.port,
				statsComp.displayPing, statsComp.displayJitter, estbuff,
				statsComp.displayInputBufferSize, statsComp.displayIdealInputBufferSize,
				statsComp.displaySentTotal, statsComp.displaySentAvg,
				statsComp.displayRecvTotal, statsComp.displayRecvAvg, statsComp.displayLoss,
				conn.getPacketSendBudget()
			);

			{
				const float32 r1 = conn.getPacketRecvRate();
				auto r2 = r1;
				ImGui::SliderFloat("Packet Recv Rate", &r2, 1.0f, 256.0f);

				if (r2 != r1) {
					if (auto msg = conn.beginMessage<MessageType::CONFIG_NETWORK>()) {
						conn.setPacketRecvRate(r2);
						msg.write(r2);
					} else {
						ENGINE_WARN("Unable to set network recv rate!");
					}
				}
			}

			const auto end = Engine::Clock::Seconds{now.time_since_epoch()}.count();
			const auto begin = Engine::Clock::Seconds{rollingWindow.time_since_epoch()}.count();

			constexpr auto yAxisflags = ImPlotAxisFlags_NoGridLines;
			constexpr auto y2Axisflags = yAxisflags;
			constexpr auto xAxisflags = yAxisflags & ImPlotAxisFlags_NoTickLabels;
			constexpr auto yScale = 500.0f;
			ImGui::PushID(ent.id);
			ImPlot::SetNextPlotLimitsX(begin, end, ImGuiCond_Always);
			ImPlot::SetNextPlotLimitsY(0.0f, yScale * tickrate * 0.333f, ImGuiCond_Once, 0);
			ImPlot::SetNextPlotLimitsY(0.0f, yScale, ImGuiCond_Once, 1);
			if (ImPlot::BeginPlot(
				"##Netgraph", nullptr, nullptr, ImVec2(-1,200),
				ImPlotFlags_YAxis2,
				xAxisflags, yAxisflags, y2Axisflags)) {

				// ImGui doesn't handle color correctly so we need to convert it
				constexpr auto g = [](float32 in){ return powf(in/255.0f, 2.2f); };
				static const ImVec4 colors[] = {
					{g(239), g( 91), g( 91), 1.0f},
					{g( 32), g(163), g(158), 1.0f},
					{g(255), g(186), g( 73), 1.0f},
					//{g(220), g(214), g(247), 0.33f},
					//{g(199), g(242), g(167), 0.33f},
					{g(219), g(254), g(184), 0.33f},
				};

				ImPlot::SetPlotYAxis(0);
				// TODO: update - ImPlot::SetColormap(colors, sizeof(colors));
				// TODO: thickness?
				ImPlot::PlotLineG("Avg Sent (Bytes / Second)", netGetPointAvg<0>, &buff, buff.size(), 0);
				ImPlot::PlotLineG("Avg Recv (Bytes / Second)", netGetPointAvg<1>, &buff, buff.size(), 0);

				ImPlot::SetPlotYAxis(1);
				ImPlot::PlotBarsG("Sent (Bytes)", netGetDiff<0>, &buff, buff.size(), 1.0f / tickrate, 0);
				ImPlot::PlotBarsG("Recv (Bytes)", netGetDiff<1>, &buff, buff.size(), 1.0f / tickrate, 0);

				ImPlot::EndPlot();
				// TODO: update - ImPlot::SetColormap(ImPlotColormap_Default);
				ImGui::Separator();
			}
			ImGui::PopID();
		}
	}

	template<bool B>
	ImPlotPoint UISystem::netGetPointAvg(void* data, int idx) {
		const auto& buff = *reinterpret_cast<decltype(NetworkStatsComponent::buffer)*>(data);
		const auto& stats = buff[idx];
		return {
			Engine::Clock::Seconds{stats.time.time_since_epoch()}.count(),
			B ? stats.recv.avg : stats.sent.avg
		};
	};

	template<bool B>
	ImPlotPoint UISystem::netGetDiff(void* data, int idx) {
		const auto& buff = *reinterpret_cast<decltype(NetworkStatsComponent::buffer)*>(data);
		const auto& stats = buff[idx];
		return {
			Engine::Clock::Seconds{stats.time.time_since_epoch()}.count(),
			B ? stats.recv.diff : stats.sent.diff
		};
	}

	void UISystem::ui_entities() {
		if (!ImGui::CollapsingHeader("Entities")) { return; }

		for (const auto& [ent, state] : world.getEntities()) {
			if (!world.isAlive(ent)) { continue; }
			ImGui::PushID(ent.id);

			if (ImGui::TreeNode("Title", "Entity(%i, %i)", ent.id, ent.gen)) {
				constexpr auto size = Engine::ECS::ComponentBitset::capacity();
				static char comps[size] = {};
				const auto& bits = world.getComponentsBitset(ent);

				for (int i = 0; i < size; ++i) {
					comps[i] = bits.test(size - i - 1) ? '1' : '0';
				}

				// TODO: more detailed component inspection

				ImGui::LabelText("", "Components: %s", comps);
				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}

	void UISystem::ui_nethealth() {
		if (!ImGui::CollapsingHeader("Network Health")) { return; }
		int connection = 0;

		for (auto ent : world.getFilter<ConnectionComponent>()) {
			auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
			const auto& addr = conn.address();

			if (connection) {
				ImGui::Separator();
			}

			ImGui::Text("%i.%i.%i.%i:%i", addr.a, addr.b, addr.c, addr.d, addr.port);
			for (int32 c = 0; true; ++c) {
				const auto s = conn.getChannelQueueSize(c);
				if (s < 0) { break; }
				ImGui::Text("Channel%i: %i", c, s);
			}

			++connection;
		}
	}
}
