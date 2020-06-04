// Windows
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

// STD
#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>
#include <filesystem>
#include <regex>
#include <iterator>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// SOIL
#include <soil/SOIL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>
#include <Engine/Camera.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Win32/Win32.hpp>
#include <Engine/WindowCallbacks.hpp>
#include <Engine/Window.hpp>
#include <Engine/ImGui/ImGui.hpp>
#include <Engine/Input/InputSequence.hpp>
#include <Engine/CommandLine/Parser.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/World.hpp>
#include <Game/CharacterSpellActionListener.hpp>
#include <Game/CharacterMovementActionListener.hpp>
#include <Game/MapGenerator.hpp>


namespace {
	using namespace Engine::Types;
	constexpr int32 OPENGL_VERSION_MAJOR = 4;
	constexpr int32 OPENGL_VERSION_MINOR = 5;
	GLuint mapTexture = 0;
	double avgDeltaTime = 0.0;
	
	void mapTest() {
		struct Color {
			uint8_t r = 255;
			uint8_t g = 0;
			uint8_t b = 0;

			Color() = default;
			explicit Color(uint32_t value) {
				*this = value;
			}

			Color& operator=(uint32_t value) {
				r = (value & 0x00FF0000) >> 16;
				g = (value & 0x0000FF00) >> 8;
				b = (value & 0x000000FF) >> 0;
				return *this;
			}

			void gray(uint8_t value) {
				r = g = b = value;
			}
		};

		constexpr int w = 512;
		constexpr int h = 512;
		//constexpr int w = 128;
		//constexpr int h = 128;

		Color map[h][w];

		Engine::Noise::OpenSimplexNoise simplex{1234};
		Engine::Noise::WorleyNoise worley{1234};
		Engine::Noise::WorleyNoiseFrom<&Engine::Noise::constant1> worley1{1234};
		Game::MapGenerator<
			Game::BiomeA,
			//Game::BiomeB,
			Game::BiomeC
		> mgen{1234};

		/*{
			srand((unsigned)time(NULL));
			Engine::Noise::WorleyNoise worley{rand()};
			int w = 1 << 10;
			int h = w;
			double max = 0;
			double min = 0;
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					const auto v = (double)worley.at((float)x,(float)y);
					max = std::max(max, v);
					min = std::min(min, v);
				}
			}

			// 1.08485
			std::cout << "Max: " << max << "\n";
			std::cout << "Min: " << min << "\n";
		}*/


		const auto gradient = [](float v, int y, int min, int max, float from, float to){
			if (y < min || y >= max) { return v; }
			float p = static_cast<float>(y - min) / static_cast<float>(max - min); // Get precent
			return v + (p * (to - from) + from); // Map from [0, 1] to [from, to]
		};

		const auto fill = [](float v, int y, int min, int max, float fv){
			if (y < min || y >= max) { return v; }
			return fv;
		};

		const auto begin = std::chrono::high_resolution_clock::now();
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				float v = 0.0f;
				
				{ // Cave structure
					float s = 0.051f;

					//s *= 2;
					//v += simplex.value(x * s, y * s) / 2;
					
					//s *= 2;
					//v += simplex.value(x * s, y * s) / 4;
					//
					//s *= 2;
					//v += simplex.value(x * s, y * s) / 8;
					//
					//s *= 2;
					//v += simplex.value(x * s, y * s) / 16;
				}

				/*
				{ // Surface
					constexpr int begin = 100;
					constexpr int mid = begin + 40;
					constexpr int end = mid + 30;
					v = gradient(v, y, begin, mid, -1.0f, 1.0f);
					v = gradient(v, y, mid, end, 1.0f, 0.0f);
					v = fill(v, y, 0, begin, -1.0f);
				}
				*/
				
				{ // Worley testing
					float s = 0.01f;
					//v = sqrt(worley.value(x * s, y * s).distanceSquared);
					//v = sqrt(worley.valueD2(x * s, y * s).value);
					//v = worley1.valueF2F1(x * s, y * s).value;
					v = mgen.value(x, y);
				}

				// Step
				v = v < 0.0f ? -1.0f : 1.0f;

				// Convert to color map
				const auto y2 = h - y - 1;
				map[y2][x].gray(static_cast<uint8_t>(roundf(std::max(std::min(
					(v + 1.0f) * 0.5f * 255.0f
					//v * 255.0f
					//(1 - v) * 255.0f
				, 255.0f), 0.0f))));

				if (x < 10 && y < 20) {
					map[y2][x].r = 255;
					map[y2][x].g = 0;
					map[y2][x].b = 0;
				}
			}
		}

		const auto end = std::chrono::high_resolution_clock::now();

		std::cout << "Map Time (ms): " << std::chrono::duration<long double, std::milli>{end - begin}.count() << "\n";

		glGenTextures(1, &mapTexture);
		glBindTexture(GL_TEXTURE_2D, mapTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, map);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void initializeOpenGL() {
		auto loaded = ogl_LoadFunctions();

		if (loaded == ogl_LOAD_FAILED) {
			ENGINE_ERROR("[glLoadGen] initialization failed.");
		}

		auto failed = loaded - ogl_LOAD_SUCCEEDED;
		if (failed > 0) {
			ENGINE_ERROR("[glLoadGen] Failed to load ", failed, " functions.");
		}


		if (!ogl_IsVersionGEQ(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)) {
			ENGINE_ERROR("[glLoadGen] OpenGL version ", OPENGL_VERSION_MAJOR, ".", OPENGL_VERSION_MINOR, " is not available.");
		}
	}

	b2Body* createPhysicsSquare(Engine::ECS::Entity ent, Game::PhysicsSystem& physSys, b2Vec2 position = b2Vec2_zero) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = position;

		b2Body* body = physSys.createBody(ent, bodyDef);

		b2PolygonShape shape;
		shape.SetAsBox(1.0f/8, 1.0f/8);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
		body->SetLinearDamping(10.0f);
		body->SetFixedRotation(true);

		return body;
	}
}

namespace {
	// TODO: once input is finished this should go away;
	struct TempWorldEngineWrapper {
		Engine::EngineInstance& engine;
		Game::World& world;
	};

	// TODO: move into file
	class WindowCallbacks final : public Engine::WindowCallbacks<TempWorldEngineWrapper> {
		void resizeCallback(int32 w, int32 h) override {
			glViewport(0, 0, w, h);
			userdata->engine.camera.setAsOrtho(w, h, 1.0f / 250.0f);
		}

		void keyCallback(Engine::Input::InputEvent event) override {
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::keyCallback(event.state);
		}

		void charCallback(wchar_t character) {
			Engine::ImGui::charCallback(character);
		}

		void mouseButtonCallback(Engine::Input::InputEvent event) override {
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::mouseButtonCallback(event.state);
		}

		void mouseWheelCallback(float32 x, float32 y) override {
			Engine::ImGui::scrollCallback(x, y);
		}

		void mouseMoveCallback(Engine::Input::InputEvent event) override {
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::mouseMoveCallback(event.state);
		}

		void mouseLeaveCallback() override {
		}

		void mouseEnterCallback() override {
			Engine::ImGui::mouseEnterCallback();
		}
	};
}

void run(int argc, char* argv[]) {
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Init networking
	Engine::Net::startup();

	////////////////////////////////////////////////////////////////////////////////////////////////
	WindowCallbacks windowCallbacks;
	Engine::Window window{{
			.colorBits = 24,
			.alphaBits = 8,
			.depthBits = 24,
			.stencilBits = 8,
		}, {
			.majorVersion = OPENGL_VERSION_MAJOR,
			.minorVersion = OPENGL_VERSION_MINOR,
			#ifdef DEBUG
				.debug = true,
			#else
				.debug = false,
			#endif
		},
		windowCallbacks
	};
	window.makeContextCurrent();
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialize OpenGL functions
	initializeOpenGL();
	
	// OpenGL debug message
	#if defined(DEBUG_GRAPHICS)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Engine::Debug::GL::debugMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	#endif

	glEnable(GL_FRAMEBUFFER_SRGB);
	////////////////////////////////////////////////////////////////////////////////////////////////
	// UI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	Engine::ImGui::init(window);
	ImGui::StyleColorsDark();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Engine
	Engine::EngineInstance engine;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Command Line
	{
		using namespace Engine::CommandLine;
		using namespace Engine::Net;
		auto& parser = engine.commandLineArgs;

		parser
			.add<uint16>("port", 'p', 21212,
				"The port to listen on.")
			.add<IPv4Address>("group", 'g', {224,0,0,212, 21212},
				"The multicast group to join for server discovery.")
			.add<std::string>("log", 'l', "", // TODO: impl
				"The file to use for logging.")
		;

		parser.parse(argc - 1, argv + 1);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// World
	auto worldStorage = std::make_unique<Game::World>(1.0f / 60.0f, engine);
	Game::World& world = *worldStorage.get();
	TempWorldEngineWrapper wrapper{engine, world};
	windowCallbacks.userdata = &wrapper;
	const auto player = world.createEntity();
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Binds
	{
		using namespace Engine::Input;
		auto& im = engine.inputManager;
		auto& as = world.getSystem<Game::ActionSystem>();

		const auto Spell_1 = as.create("Spell_1");
		const auto Move_Up = as.create("Move_Up");
		const auto Move_Down = as.create("Move_Down");
		const auto Move_Left = as.create("Move_Left");
		const auto Move_Right = as.create("Move_Right");
		const auto Edit_Place = as.create("Edit_Place");
		const auto Edit_Remove = as.create("Edit_Remove");
		const auto Target_X = as.create("Target_X");
		const auto Target_Y = as.create("Target_Y");
		
		if constexpr (ENGINE_CLIENT) {
			const auto& filter = world.getFilterFor<Game::ActivePlayerFlag>();
			const auto pa = [&](auto action, auto curr){
				for (auto ent : filter) {
					as.processAction({ent, action, curr});
				}
			};

			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 29}, // CTRL
				InputId{InputType::KEYBOARD, 1, 46}, // C
				}, [&](Value curr, Value prev){ pa(Spell_1, curr); });
			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 29}, // CTRL
				InputId{InputType::KEYBOARD, 1, 56}, // ALT
				InputId{InputType::KEYBOARD, 1, 16}, // Q
			}, [&](Value curr, Value prev){ pa(Spell_1, curr); });
			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 57}
			}, [&](Value curr, Value prev){ pa(Spell_1, curr); });
			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 17}
			}, [&](Value curr, Value prev){ pa(Move_Up, curr); });
			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 31}
			}, [&](Value curr, Value prev){ pa(Move_Down, curr); });
			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 30}
			}, [&](Value curr, Value prev){ pa(Move_Left, curr); });
			im.addBind(InputSequence{
				InputId{InputType::KEYBOARD, 1, 32}
			}, [&](Value curr, Value prev){ pa(Move_Right, curr); });
			im.addBind(InputSequence{
				InputId{InputType::MOUSE, 0, 0}
			}, [&](Value curr, Value prev){ pa(Edit_Place, curr); });
			im.addBind(InputSequence{
				InputId{InputType::MOUSE, 0, 1}
			}, [&](Value curr, Value prev){ pa(Edit_Remove, curr); });
			im.addBind(InputSequence{
				InputId{InputType::MOUSE_AXIS, 0, 0}
			}, [&](Value curr, Value prev){ pa(Target_X, curr); });
			im.addBind(InputSequence{
				InputId{InputType::MOUSE_AXIS, 0, 1}
			}, [&](Value curr, Value prev){ pa(Target_Y, curr); });
		}

		as.addListener(Spell_1, Game::CharacterSpellActionListener{engine, world});
		as.addListener(Move_Up, Game::CharacterMovementActionListener{world, glm::ivec2{0, 1}});
		as.addListener(Move_Down, Game::CharacterMovementActionListener{world, glm::ivec2{0, -1}});
		as.addListener(Move_Left, Game::CharacterMovementActionListener{world, glm::ivec2{-1, 0}});
		as.addListener(Move_Right, Game::CharacterMovementActionListener{world, glm::ivec2{1, 0}});
		as.addListener(Edit_Remove, [&](Engine::ECS::Entity ent, ActionId, Value curr, Value prev){ world.getComponent<Game::MapEditComponent>(ent).remove = curr && !prev; return false; });
		as.addListener(Edit_Place, [&](Engine::ECS::Entity ent, ActionId, Value curr, Value prev){ world.getComponent<Game::MapEditComponent>(ent).place = curr && !prev; return false; });

		if constexpr (ENGINE_CLIENT) {
			{ // TODO: better way to do this
				auto& netSys = world.getSystem<Game::NetworkingSystem>();
				auto& connFilter = world.getFilterFor<Game::ConnectionComponent>();
				
				const auto sendAction = [&](Engine::ECS::Entity ent, ActionId aid, Value curr, Value prev){
					ENGINE_DEBUG_ASSERT(connFilter.size() <= 1);
					for (auto& ply : connFilter) {
						auto& conn = *world.getComponent<Game::ConnectionComponent>(ply).conn;
						conn.writer.next(Game::MessageType::ACTION, Engine::Net::Channel::UNRELIABLE);
						conn.writer.write(aid);
						conn.writer.write(curr);
						//std::cout << "Send action: " << ent << " - " << aid << " - " << curr.value << " - " << connFilter.size() << "\n";
					}
					return false;
				};
				
				as.addListener(Spell_1, sendAction);
				as.addListener(Move_Up, sendAction);
				as.addListener(Move_Down, sendAction);
				as.addListener(Move_Left, sendAction);
				as.addListener(Move_Right, sendAction);
				as.addListener(Edit_Remove, sendAction);
				as.addListener(Edit_Place, sendAction);
			}
		}
	}

	// More engine stuff
	#if defined (DEBUG_PHYSICS)
		world.getSystem<Game::PhysicsSystem>().getDebugDraw().setup(engine.camera);
	#endif

	{ // TODO: is there a better way to handle these setup functions? This seems dumb.
		//auto& physSys = world.getSystem<Game::PhysicsSystem>();
		//
		//// Player
		//ENGINE_LOG("PlayerId: ", player);
		//world.addComponent<Game::PlayerComponent>(player);
		//world.addComponent<Game::MapEditComponent>(player);
		//world.addComponent<Game::SpriteComponent>(player).texture = engine.textureManager.get("../assets/player.png");
		//world.addComponent<Game::PhysicsComponent>(player).setBody(physSys.createPhysicsCircle(player));
		//world.addComponent<Game::CharacterMovementComponent>(player);
		//world.addComponent<Game::CharacterSpellComponent>(player);
		//world.addComponent<Game::ActivePlayerComponent>(player);
		//
		//// TODO: cleaner way to do this. constructor args?
		//world.addComponent<Game::ActionComponent>(player).grow(world.getSystem<Game::ActionSystem>().count());
	}

	//// ImGui callbacks
	// TODO: glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
	// TODO: glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

	// Procedural test
	//mapTest();
	// Main loop
	std::array<float, 64> deltas = {};
	size_t deltaIndex = 0;
	window.show();
	window.center();
	while (!window.shouldClose()) {
		window.poll();

		// Rendering
		glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ECS
		world.run();

		// Frame rate
		deltas[deltaIndex] = world.getDeltaTime();
		deltaIndex = ++deltaIndex % deltas.size();
		if (deltaIndex == 0) {
			avgDeltaTime = std::accumulate(deltas.cbegin(), deltas.cend(), 0.0) / deltas.size();
		}

		// Physics debug
		#if defined (DEBUG_PHYSICS)
			world.getSystem<Game::PhysicsSystem>().getDebugDraw().draw();
		#endif

		window.swapBuffers();
		//std::this_thread::sleep_for(std::chrono::milliseconds{250});
	}

	glDeleteTextures(1, &mapTexture);

	// UI cleanup
	Engine::ImGui::shutdown();
	ImGui::DestroyContext();

	// Network cleanup
	Engine::Net::shutdown();
}

static_assert(ENGINE_CLIENT ^ ENGINE_SERVER, "Must be either client or server");
int entry(int argc, char* argv[]) {
	if(!AllocConsole()) {
		ENGINE_ERROR("Unable to allocate console window - ", Engine::Win32::getLastErrorMessage());
	} else {
		FILE* unused;
		freopen_s(&unused, "CONIN$", "r", stdin);
		freopen_s(&unused, "CONOUT$", "w", stdout);
		freopen_s(&unused, "CONOUT$", "w", stderr);
	}

	const auto console = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!SetConsoleMode(console, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
		ENGINE_WARN(Engine::Win32::getLastErrorMessage());
	}

	ENGINE_LOG("This is a test of the", "console ouput");
	ENGINE_INFO("This is a test of the", "console ouput");
	ENGINE_SUCCESS("This is a test of the", "console ouput");
	ENGINE_WARN("This is a test of the", "console ouput");
	//ENGINE_ERROR("This is a test of the", "console ouput");

	std::atexit([](){
	});

	{ // Position the console
		auto window = GetConsoleWindow();

		if constexpr (ENGINE_CLIENT) {
			SetWindowPos(window, HWND_TOP, 0, 0, 1000, 500, 0);
			SetWindowTextW(window, L"Client");
		} else if constexpr (ENGINE_SERVER) {
			SetWindowPos(window, HWND_TOP, 0, 500, 1000, 500, 0);
			SetWindowTextW(window, L"Server");
		}
	}

	ENGINE_INFO("Working Directory: ", std::filesystem::current_path().generic_string());

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// At seemingly random the debugger decides to not work for STL code. Enable, run, disable, run seems to fix this for some reason.
	// Other times spam clicking on Visual Studio while the program launches fixes this. 10/10.
	// --------------------------
	// Function Breakpoints: assert, _wassert, abort, exit
	// --------------------------
	//_set_error_mode(_OUT_TO_MSGBOX);
	//_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	run(argc, argv);

	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}

#ifdef ENGINE_OS_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
	return entry(__argc, __argv);
}
#else
int main(int argc, char* argv[]) {
	return entry(argc, argv);
}
#endif
