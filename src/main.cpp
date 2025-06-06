// STD
#include <filesystem>
#include <random>
#include <type_traits>
#include <thread>

// Engine
#include <Engine/engine.hpp>
#include <Engine/Logger.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/Noise/SimplexNoise.hpp>
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Win32/Win32.hpp>
#include <Engine/Window.hpp>
#include <Engine/CommandLine/Parser.hpp>
#include <Engine/Input/BindManager.hpp>
#include <Engine/Camera.hpp>

#include <Engine/UI/Context.hpp>
#include <Engine/UI/ImageDisplay.hpp>
#include <Engine/UI/Window.hpp>
#include <Engine/UI/TextBox.hpp>

// Game
#include <Game/common.hpp>
#include <Game/World.hpp>
#include <Game/MapGenerator2.hpp>
#include <Game/systems/UISystem.hpp>
#include <Game/UI/ConsoleWindow.hpp>
#include <Game/UI/MapPreview.hpp>

// FMT
#include <fmt/xchar.h>

// Win32
#if ENGINE_OS_WINDOWS
#include <timeapi.h>
#endif

void test_AreaWalker();
void setupCommands(Game::EngineInstance& engine, Engine::Window& window);
void setupBinds(Game::EngineInstance& engine);

namespace {
	using namespace Engine::Types;
	constexpr int32 OPENGL_VERSION_MAJOR = 4;
	constexpr int32 OPENGL_VERSION_MINOR = 5;
	double avgDeltaTime = 0.0;

	static Engine::Clock::TimePoint launchTime;
	static Engine::Clock::TimePoint startTime;

	struct {
		constexpr static int w = 512;
		constexpr static int h = 512;
		Engine::Gfx::Texture2D tex2d;
		//GLuint tex = 0;
		//GLuint tex2 = 0;
		//float32 data2[w] = {};
		//float32 data[w] = {};
	} map;
	
	void mapTest(float32 xOffset = 0, float32 yOffset = 0, float32 xZoom = 1.0f, float32 yZoom = 1.0f) {
		struct Color {
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;

			Color() = default;
			Color(int r, int g, int b) : r{static_cast<uint8_t>(r)}, g{static_cast<uint8_t>(g)}, b{static_cast<uint8_t>(b)} {}
			explicit Color(uint32_t value) { *this = value; }

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

		Color data[map.h][map.w];
		//Color data2[map.h][map.w];
		
		/*ENGINE_INLINE_CALLS {
			std::ios_base::sync_with_stdio(false);
			Engine::Noise::SimplexNoise simplex{1234};
			//Engine::Noise::OpenSimplexNoise simplex{1234};
			constexpr float32 r = 100000;
			float32 min = FLT_MAX;
			float32 max = FLT_MIN;
			uint8 p = 0;

			float32 scales[] = {
				11.168912f,
				11.1234f,
				11.123f,
				11.12f,
				11.1f,
				11.0f,
				10.0f,
				7.0f,
				5.0f,
				3.0f,
				2.0f,
				1.0f,
				0.75f,
				0.71f,
				0.701f,
				0.7f,
				0.5f,
				0.4f,
				0.3f,
				0.2f,
				0.1f,
				0.011f,
				0.0013f,
				0.0012f,
				0.0006f,
				0.0005f,
				0.0004f,
				0.0003f,
				0.0002f,
				0.0001f,
			};

			for (const auto s : scales) {
				for (float32 x = -r; x < r; ++x) {
					//for (float32 y = -r; y < r; ++y) {
					//	auto v = simplex.value(x*s, y*s);
					//	min = v < min ? v : min;
					//	max = v > max ? v : max;
					//}

					auto v = simplex.value1D(x * s);
					min = v < min ? v : min;
					max = v > max ? v : max;

					if (!++p) { ENGINE_INFO("Range: [", min, ", ", max, "] @ ", s); }
				}
			}
			ENGINE_INFO("\n\n===================================================");
			ENGINE_INFO("Range: [", min, ", ", max, "]");
			ENGINE_INFO("===================================================\n");
			std::ios_base::sync_with_stdio(true);
		}/**/

		//using Perm = Engine::Noise::RangePermutation<256>;
		//using Dist = decltype([](auto...){ return 1; });
		//Engine::Noise::WorleyNoiseGeneric<Perm, Dist> worley2{
		//	std::piecewise_construct,
		//	std::forward_as_tuple(1234),
		//	std::forward_as_tuple()
		//};
		

		const Engine::Noise::RangePermutation<256> realPerm = 1234;
		auto perm = [&](auto... as){ return realPerm(as...); };
		//auto dist = [](auto...) { return 1; }; // TODO: cosntexpr SFINAE possible with consteval?
		auto dist = Engine::Noise::ConstantDistribution<1>{};
		auto metric = Engine::Noise::MetricEuclidean2{};
		//auto metric = Engine::Noise::MetricManhattan{};
		//auto metric = Engine::Noise::MetricChebyshev{};
		Engine::Noise::WorleyNoiseGeneric worley2{perm, dist, metric};
		//Engine::Noise::WorleyNoise2 worley2{1234556};
		
		//Engine::Noise::WorleyNoiseFrom<&Engine::Noise::constant1> worley1{1234};
		Engine::Noise::SimplexNoise simplex{(uint64)(srand((uint32)time(0)), rand())};
		Engine::Noise::OpenSimplexNoise simplex2{(uint64)(srand((uint32)time(0)), rand())};
		Game::MapGenerator2 mgen{12345};

		const auto gradient = [](float v, int y, int min, int max, float from, float to){
			if (y < min || y >= max) { return v; }
			float p = static_cast<float>(y - min) / static_cast<float>(max - min); // Get precent
			return v + (p * (to - from) + from); // Map from [0, 1] to [from, to]
		};

		const auto fill = [](float v, int y, int min, int max, float fv){
			if (y < min || y >= max) { return v; }
			return fv;
		};

		Color blockToColor[Game::BlockId::_count] = {};
		blockToColor[Game::BlockId::Entity]	= {0, 120, 189};
		blockToColor[Game::BlockId::Debug]	= {255, 0, 0};
		blockToColor[Game::BlockId::Debug2]	= {200, 26, 226};
		blockToColor[Game::BlockId::Debug3]	= {226, 26, 162};
		blockToColor[Game::BlockId::Debug4]	= {226, 26, 111};
		blockToColor[Game::BlockId::Dirt]	= {158, 98, 33};
		blockToColor[Game::BlockId::Grass]	= {67, 226, 71};
		blockToColor[Game::BlockId::Iron]	= {144, 144, 144};
		blockToColor[Game::BlockId::Gold]	= {255, 235, 65};

		Game::MapGenerator2::BlockGenData discard;

		const auto begin = std::chrono::high_resolution_clock::now();
		for (int y = 0; y < map.h; ++y) {
			for (int x = 0; x < map.w; ++x) {
				const float32 xm = (x - map.w/2) * xZoom + xOffset;
				const float32 ym = (y - map.h/2) * yZoom + yOffset;

				if constexpr (true) {
					const auto v = mgen.value(static_cast<int32>(xm), static_cast<int32>(ym), discard);
					data[y][x] = blockToColor[v];
				}

				if constexpr (false) {
					constexpr float32 ps = 0.03f;
					auto p = 50*simplex2.value(ps*x,ps*y);
					//p += 10*simplex2.value(4*ps*x,4*ps*y);

					glm::vec2 pos = glm::vec2{x+p, y+p};
					constexpr glm::vec2 center = {map.w * 0.5f, map.h * 0.5f};
					const auto diff = center - pos;

					constexpr float32 r = 5.5f;
					const auto off = r*glm::normalize(diff);

					auto v = glm::length(diff);

					float32 f = 0.75f;
					float32 l = 1 / f;
					constexpr float32 i = 32;

					v += i * l * simplex2.value(f * off.x, f * off.y);
					f *= 2.0f;
					l *= 0.5f;
					v += i * l * simplex2.value(f * off.x, f * off.y);
					f *= 2.0f;
					l *= 0.5f;

					v += 30*simplex2.value(0.1f*x, 0.1f*y);
					v += 15*simplex2.value(0.3f*x, 0.3f*y);

					v = (v < 150) * 255.0f;
					//v = v / 300;
					data[y][x].gray(static_cast<uint8>(std::clamp(v, 0.0f, 255.0f)));
				}

				if constexpr (false) {
					const auto off1 = simplex2.value(xm * 0.021f, ym * 0.021f);
					const float32 o1 = 0.4f;
					const auto x2 = xm * 0.01f + off1 * o1;
					const auto y2 = ym * 0.01f + off1 * o1;

					auto m = simplex2.value(xm*0.3f, ym*0.3f);
					//m += simplex2.value(xm*0.15f, ym*0.15f);
					const auto wv = worley2.valueF2F1(x2, y2);
					auto w = wv.value * (1.0f/0.8f);
					auto v = w - m * 0.15f;
					auto a = (uint8)std::clamp(
						(v > 0.2f ? 1.0f : 0.0f)
						* 255.0f
					, 0.0f, 255.0f);
					//w += 0.2f;
					data[y][x].r = perm(wv.cell.x, wv.cell.y) * a;
					data[y][x].g = perm(wv.cell.y, wv.cell.x) * a;
					data[y][x].b = perm(wv.cell.x + 5, wv.cell.y - 5) * a;
				}
				
				//map.data[x] = simplex.value1D(x * 0.1f);
				//map.data2[x] = simplex2.scaled(x * 0.15f, 42.0f);
				//
				//data[y][x].gray((uint8)std::clamp((1 + map.data[x]) * 0.5f * 255.0f, 0.0f, 255.0f));
				//data2[y][x].gray((uint8)std::clamp((1 + map.data2[x]) * 0.5f * 255.0f, 0.0f, 255.0f));
			}
		}
		const auto end = std::chrono::high_resolution_clock::now();

		std::cout << "Map Time (ms): " << std::chrono::duration<long double, std::milli>{end - begin}.count() << "\n";

		using namespace Engine::Gfx;
		if (!map.tex2d) {
			map.tex2d.setStorage(TextureFormat::SRGB8, {map.w, map.h});
		}

		map.tex2d.setFilter(TextureFilter::Nearest);
		map.tex2d.setWrap(TextureWrap::Repeat);
		map.tex2d.setSubImage(0, {}, {map.w, map.h}, PixelFormat::RGB8, data);
	}

	void performExit(const char* reason) {
		ENGINE_LOG("Shutting down: ", reason, "\n\n");
	};
	
	// TODO: add actual actual unit test once gtest is fixed
	void test_divFloor() {
		using Engine::Math::divFloor;

		{ auto a = divFloor(0, 1); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divFloor(0, 2); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divFloor(0, 3); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divFloor(0, 4); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divFloor(0, 5); assert(a.q == 0); assert(a.r == 0); }

		{ auto a = divFloor(1, 1); assert(a.q == 1); assert(a.r == 0); }
		{ auto a = divFloor(1, 2); assert(a.q == 0); assert(a.r == 1); }
		{ auto a = divFloor(1, 3); assert(a.q == 0); assert(a.r == 1); }
		{ auto a = divFloor(1, 4); assert(a.q == 0); assert(a.r == 1); }
		{ auto a = divFloor(1, 5); assert(a.q == 0); assert(a.r == 1); }

		{ auto a = divFloor(2, 1); assert(a.q == 2); assert(a.r == 0); }
		{ auto a = divFloor(2, 2); assert(a.q == 1); assert(a.r == 0); }
		{ auto a = divFloor(2, 3); assert(a.q == 0); assert(a.r == 2); }
		{ auto a = divFloor(2, 4); assert(a.q == 0); assert(a.r == 2); }
		{ auto a = divFloor(2, 5); assert(a.q == 0); assert(a.r == 2); }

		{ auto a = divFloor(3, 1); assert(a.q == 3); assert(a.r == 0); }
		{ auto a = divFloor(3, 2); assert(a.q == 1); assert(a.r == 1); }
		{ auto a = divFloor(3, 3); assert(a.q == 1); assert(a.r == 0); }
		{ auto a = divFloor(3, 4); assert(a.q == 0); assert(a.r == 3); }
		{ auto a = divFloor(3, 5); assert(a.q == 0); assert(a.r == 3); }

		{ auto a = divFloor(4, 1); assert(a.q == 4); assert(a.r == 0); }
		{ auto a = divFloor(4, 2); assert(a.q == 2); assert(a.r == 0); }
		{ auto a = divFloor(4, 3); assert(a.q == 1); assert(a.r == 1); }
		{ auto a = divFloor(4, 4); assert(a.q == 1); assert(a.r == 0); }
		{ auto a = divFloor(4, 5); assert(a.q == 0); assert(a.r == 4); }

		{ auto a = divFloor(5, 1); assert(a.q == 5); assert(a.r == 0); }
		{ auto a = divFloor(5, 2); assert(a.q == 2); assert(a.r == 1); }
		{ auto a = divFloor(5, 3); assert(a.q == 1); assert(a.r == 2); }
		{ auto a = divFloor(5, 4); assert(a.q == 1); assert(a.r == 1); }
		{ auto a = divFloor(5, 5); assert(a.q == 1); assert(a.r == 0); }
		
		{ auto a = divFloor(-1, 1); assert(a.q == -1); assert(a.r == 0); }
		{ auto a = divFloor(-1, 2); assert(a.q == -1); assert(a.r == 1); }
		{ auto a = divFloor(-1, 3); assert(a.q == -1); assert(a.r == 2); }
		{ auto a = divFloor(-1, 4); assert(a.q == -1); assert(a.r == 3); }
		{ auto a = divFloor(-1, 5); assert(a.q == -1); assert(a.r == 4); }

		{ auto a = divFloor(-2, 1); assert(a.q == -2); assert(a.r == 0); }
		{ auto a = divFloor(-2, 2); assert(a.q == -1); assert(a.r == 0); }
		{ auto a = divFloor(-2, 3); assert(a.q == -1); assert(a.r == 1); }
		{ auto a = divFloor(-2, 4); assert(a.q == -1); assert(a.r == 2); }
		{ auto a = divFloor(-2, 5); assert(a.q == -1); assert(a.r == 3); }

		{ auto a = divFloor(-3, 1); assert(a.q == -3); assert(a.r == 0); }
		{ auto a = divFloor(-3, 2); assert(a.q == -2); assert(a.r == 1); }
		{ auto a = divFloor(-3, 3); assert(a.q == -1); assert(a.r == 0); }
		{ auto a = divFloor(-3, 4); assert(a.q == -1); assert(a.r == 1); }
		{ auto a = divFloor(-3, 5); assert(a.q == -1); assert(a.r == 2); }

		{ auto a = divFloor(-4, 1); assert(a.q == -4); assert(a.r == 0); }
		{ auto a = divFloor(-4, 2); assert(a.q == -2); assert(a.r == 0); }
		{ auto a = divFloor(-4, 3); assert(a.q == -2); assert(a.r == 2); }
		{ auto a = divFloor(-4, 4); assert(a.q == -1); assert(a.r == 0); }
		{ auto a = divFloor(-4, 5); assert(a.q == -1); assert(a.r == 1); }

		{ auto a = divFloor(-5, 1); assert(a.q == -5); assert(a.r == 0); }
		{ auto a = divFloor(-5, 2); assert(a.q == -3); assert(a.r == 1); }
		{ auto a = divFloor(-5, 3); assert(a.q == -2); assert(a.r == 1); }
		{ auto a = divFloor(-5, 4); assert(a.q == -2); assert(a.r == 3); }
		{ auto a = divFloor(-5, 5); assert(a.q == -1); assert(a.r == 0); }
	}
	
	// TODO: add actual actual unit test once gtest is fixed
	void test_divCeil() {
		using Engine::Math::divCeil;

		{ auto a = divCeil(0, 1); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divCeil(0, 2); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divCeil(0, 3); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divCeil(0, 4); assert(a.q == 0); assert(a.r == 0); }
		{ auto a = divCeil(0, 5); assert(a.q == 0); assert(a.r == 0); }

		{ auto a = divCeil(1, 1); assert(a.q == 1); assert(a.r ==  0); }
		{ auto a = divCeil(1, 2); assert(a.q == 1); assert(a.r == -1); }
		{ auto a = divCeil(1, 3); assert(a.q == 1); assert(a.r == -2); }
		{ auto a = divCeil(1, 4); assert(a.q == 1); assert(a.r == -3); }
		{ auto a = divCeil(1, 5); assert(a.q == 1); assert(a.r == -4); }

		{ auto a = divCeil(2, 1); assert(a.q == 2); assert(a.r ==  0); }
		{ auto a = divCeil(2, 2); assert(a.q == 1); assert(a.r ==  0); }
		{ auto a = divCeil(2, 3); assert(a.q == 1); assert(a.r == -1); }
		{ auto a = divCeil(2, 4); assert(a.q == 1); assert(a.r == -2); }
		{ auto a = divCeil(2, 5); assert(a.q == 1); assert(a.r == -3); }

		{ auto a = divCeil(3, 1); assert(a.q == 3); assert(a.r ==  0); }
		{ auto a = divCeil(3, 2); assert(a.q == 2); assert(a.r == -1); }
		{ auto a = divCeil(3, 3); assert(a.q == 1); assert(a.r ==  0); }
		{ auto a = divCeil(3, 4); assert(a.q == 1); assert(a.r == -1); }
		{ auto a = divCeil(3, 5); assert(a.q == 1); assert(a.r == -2); }

		{ auto a = divCeil(4, 1); assert(a.q == 4); assert(a.r ==  0); }
		{ auto a = divCeil(4, 2); assert(a.q == 2); assert(a.r ==  0); }
		{ auto a = divCeil(4, 3); assert(a.q == 2); assert(a.r == -2); }
		{ auto a = divCeil(4, 4); assert(a.q == 1); assert(a.r ==  0); }
		{ auto a = divCeil(4, 5); assert(a.q == 1); assert(a.r == -1); }

		{ auto a = divCeil(5, 1); assert(a.q == 5); assert(a.r ==  0); }
		{ auto a = divCeil(5, 2); assert(a.q == 3); assert(a.r == -1); }
		{ auto a = divCeil(5, 3); assert(a.q == 2); assert(a.r == -1); }
		{ auto a = divCeil(5, 4); assert(a.q == 2); assert(a.r == -3); }
		{ auto a = divCeil(5, 5); assert(a.q == 1); assert(a.r ==  0); }
		
		{ auto a = divCeil(-1, 1); assert(a.q == -1); assert(a.r ==  0); }
		{ auto a = divCeil(-1, 2); assert(a.q ==  0); assert(a.r == -1); }
		{ auto a = divCeil(-1, 3); assert(a.q ==  0); assert(a.r == -1); }
		{ auto a = divCeil(-1, 4); assert(a.q ==  0); assert(a.r == -1); }
		{ auto a = divCeil(-1, 5); assert(a.q ==  0); assert(a.r == -1); }

		{ auto a = divCeil(-2, 1); assert(a.q == -2); assert(a.r ==  0); }
		{ auto a = divCeil(-2, 2); assert(a.q == -1); assert(a.r ==  0); }
		{ auto a = divCeil(-2, 3); assert(a.q ==  0); assert(a.r == -2); }
		{ auto a = divCeil(-2, 4); assert(a.q ==  0); assert(a.r == -2); }
		{ auto a = divCeil(-2, 5); assert(a.q ==  0); assert(a.r == -2); }

		{ auto a = divCeil(-3, 1); assert(a.q == -3); assert(a.r ==  0); }
		{ auto a = divCeil(-3, 2); assert(a.q == -1); assert(a.r == -1); }
		{ auto a = divCeil(-3, 3); assert(a.q == -1); assert(a.r ==  0); }
		{ auto a = divCeil(-3, 4); assert(a.q ==  0); assert(a.r == -3); }
		{ auto a = divCeil(-3, 5); assert(a.q ==  0); assert(a.r == -3); }

		{ auto a = divCeil(-4, 1); assert(a.q == -4); assert(a.r ==  0); }
		{ auto a = divCeil(-4, 2); assert(a.q == -2); assert(a.r ==  0); }
		{ auto a = divCeil(-4, 3); assert(a.q == -1); assert(a.r == -1); }
		{ auto a = divCeil(-4, 4); assert(a.q == -1); assert(a.r ==  0); }
		{ auto a = divCeil(-4, 5); assert(a.q ==  0); assert(a.r == -4); }

		{ auto a = divCeil(-5, 1); assert(a.q == -5); assert(a.r ==  0); }
		{ auto a = divCeil(-5, 2); assert(a.q == -2); assert(a.r == -1); }
		{ auto a = divCeil(-5, 3); assert(a.q == -1); assert(a.r == -2); }
		{ auto a = divCeil(-5, 4); assert(a.q == -1); assert(a.r == -1); }
		{ auto a = divCeil(-5, 5); assert(a.q == -1); assert(a.r ==  0); }
	}

	void test_pow() {
		using Engine::Math::pow;
		static_assert(pow(0, 0) == 1);
		static_assert(pow(0, 1) == 0);
		static_assert(pow(0, 2) == 0);
		static_assert(pow(0, 3) == 0);
		static_assert(pow(0, 4) == 0);
		static_assert(pow(0, 5) == 0);

		static_assert(pow(1, 0) == 1);
		static_assert(pow(1, 1) == 1);
		static_assert(pow(1, 2) == 1);
		static_assert(pow(1, 3) == 1);
		static_assert(pow(1, 4) == 1);
		static_assert(pow(1, 5) == 1);

		static_assert(pow(2, 0) == 1);
		static_assert(pow(2, 1) == 2);
		static_assert(pow(2, 2) == 4);
		static_assert(pow(2, 3) == 8);
		static_assert(pow(2, 4) == 16);
		static_assert(pow(2, 5) == 32);

		static_assert(pow(3, 0) == 1);
		static_assert(pow(3, 1) == 3);
		static_assert(pow(3, 2) == 9);
		static_assert(pow(3, 3) == 27);
		static_assert(pow(3, 4) == 81);
		static_assert(pow(3, 5) == 243);
	}
}

// TODO: cleanup all this map this mess.
namespace Game::UI {
	using namespace Engine::UI;
	class MapPreviewDragArea : public ImageDisplay {
		private:
			glm::vec2 move = {};
			float32 zoomAccum = 0;
			Engine::Clock::TimePoint lastZoom = {};

		public:
			glm::vec2 offset = {8933, -185.000};
			glm::vec2 zoom = {0.5, 0.5};

		public:
			using ImageDisplay::ImageDisplay;

			void render() override {
				ImageDisplay::render();
				if (zoomAccum && (Engine::Clock::now() - lastZoom) > std::chrono::milliseconds{1000}) {
					lastZoom = Engine::Clock::now();
					rebuild();
				}
			}

			glm::vec2 scale() { return getSize() / glm::vec2{map.w, map.h}; }

			void rebuild() {
				if (zoomAccum) {
					auto z = std::clamp(zoomAccum * 0.2f, -0.9f, 0.9f);
					zoom -= zoom * z;
					zoom = glm::max(zoom, 0.05f);
					zoomAccum = 0;
				}
				mapTest(offset.x, offset.y, zoom.x, zoom.y);
			}

			bool onAction(ActionEvent action) override {
				switch (action) {
					case Action::Scroll: {
						zoomAccum += action.value.f32;
						return true;
					}
				}
				return false;
			}

			bool onBeginActivate() override {
				move = ctx->getCursor();
				return true;
			}

			void onEndActivate() override {
				// TODO: need to fix scale depending on sz
				move -= ctx->getCursor();
				if (move.x || move.y) {
					move.y = -move.y;
					const glm::ivec2 diff = glm::round(move * zoom / scale());
					offset += diff;
					rebuild();
				}
			}
	};

	MapPreview::MapPreview(Context* context) : Window{context} {
		auto& theme = ctx->getTheme();
		auto cont = getContent();
		// TODO: block pos tooltip

		mapTest(); // TODO: rm - temp to fix invalid texture in ImageDisplay
		area = ctx->constructPanel<MapPreviewDragArea>();
		area->setTexture(map.tex2d);

		auto sec = ctx->createPanel<Panel>(cont);
		sec->setLayout(new DirectionalLayout{Direction::Horizontal, Align::Stretch, Align::Start, theme.sizes.pad1});
		sec->setAutoSizeHeight(true);

		auto textGetter = [](auto& var){ return [&var, last=0.0f](TextBox& box) mutable {
			if (var == last) { return; }
			last = var;
			box.setText(std::to_string(last));
		};};

		auto textSetter = [area = this->area](auto& var){ return [area, &var](TextBox& box) {
			std::from_chars(std::to_address(box.getText().begin()), std::to_address(box.getText().end()), var);
			area->rebuild();
		};};
					
		xMove = ctx->createPanel<TextBox>(sec);
		xMove->autoSize();
		xMove->bind(textGetter(area->offset.x), textSetter(area->offset.x));
					
		yMove = ctx->createPanel<TextBox>(sec);
		yMove->autoSize();
		yMove->bind(textGetter(area->offset.y), textSetter(area->offset.y));
					
		xZoom = ctx->createPanel<TextBox>(sec);
		xZoom->autoSize();
		xZoom->bind(textGetter(area->zoom.x), textSetter(area->zoom.x));
					
		yZoom = ctx->createPanel<TextBox>(sec);
		yZoom->autoSize();
		yZoom->bind(textGetter(area->zoom.y), textSetter(area->zoom.y));

		sec->setFixedHeight(sec->getHeight());
		cont->setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch, Align::Stretch, theme.sizes.pad1});
		cont->addChild(area);
	}
}

namespace {
	// TODO: move into file
	class WindowCallbacks final : public Engine::WindowCallbacks {
		public:
			Game::EngineInstance* userdata;

		private:
			wchar_t buffer16[2] = {};
			char buffer8[4] = {};
			static_assert(sizeof(buffer16) == 4 && sizeof(buffer8) == 4);
			std::string_view view;

			uint32 convertBuffers(int l) {
				return static_cast<uint32>(WideCharToMultiByte(CP_UTF8, 0,
					buffer16, l,
					buffer8, 4,
					nullptr, nullptr
				));
			}
		
			void settingsChanged() override {
				userdata->getUIContext().configUserSettings();
			}

			void resizeCallback(int32 w, int32 h) override {
				ENGINE_LOG("Resize: ", w, " ", h);
				glViewport(0, 0, w, h);
				userdata->getCamera().setAsOrtho(w, h, Game::pixelRescaleFactor);
				userdata->getUIContext().onResize(w, h);
			}

			void keyCallback(Engine::Input::InputEvent event) override {
				event.state.id.device = 0;
				if (userdata->getUIContext().onKey(event)) { return; }
				userdata->getBindManager().processInput(event);
			}

			void charCallback(wchar_t ch, Engine::Input::KeyCode code) override {
				// Convert from UTF-16 to UTF-8
				// Code point that requires multiple code units (surrogate pair)
				if (ch > 0xD7FF && ch < 0xE000) {
					if (ch < 0xDC00) {
						// Don't process until we get the low surrogate
						buffer16[0] = ch;
						return;
					} else {
						buffer16[1] = ch;
					}
				
					view = std::string_view{buffer8, convertBuffers(2)};
				} else {
					buffer16[0] = ch;
					view = std::string_view{buffer8, convertBuffers(1)};
				}

				if (userdata->getUIContext().onText(view, code)) { return; }
			}

			void mouseButtonCallback(Engine::Input::InputEvent event) override {
				event.state.id.device = 0;
				if (userdata->getUIContext().onMouse(event)) { return; }
				userdata->getBindManager().processInput(event);
			}

			void mouseWheelCallback(Engine::Input::InputEvent event) override {
				event.state.id.device = 0;
				if (userdata->getUIContext().onMouseWheel(event)) { return; }
				userdata->getBindManager().processInput(event);
			}

			void mouseMoveCallback(Engine::Input::InputEvent event) override {
				event.state.id.device = 0;
				if (userdata->getUIContext().onMouseMove(event)) { return; }
				userdata->getBindManager().processInput(event);
			}

			void mouseLeaveCallback() override {
				userdata->getUIContext().onFocus(false);
			}

			void mouseEnterCallback() override {
				userdata->getUIContext().onFocus(true);
			}

			void gainFocus() override {}
			void loseFocus() override {}
	};
}

void run(int argc, char* argv[]) {
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Init networking
	////////////////////////////////////////////////////////////////////////////////////////////////
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
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Engine
	////////////////////////////////////////////////////////////////////////////////////////////////
	Game::EngineInstance engine;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Setup logging
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		auto& logger = Engine::getGlobalConfig<true>().logger;
		logger.userdata = &engine;

		logger.cleanWritter = [](const Engine::Logger& logger, const Engine::Logger::Info& info, std::string_view format, fmt::format_args args){
			fmt::memory_buffer buffer;
			auto out = std::back_inserter(buffer);

			// Don't prepend user defined levels
			if (info.level < Engine::Log::Level::User) {
				out = '[';
				buffer.append(info.label);
				out = ']';
				out = ' ';
			}

			// Write to console
			auto* engine = static_cast<Game::EngineInstance*>(logger.userdata);
			auto& uiSys = engine->getWorld().getSystem<Game::UISystem>();
			fmt::vformat_to(out, format, args);
			uiSys.getConsole()->push(std::string_view{buffer});
		};
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Resources
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		// TODO: these should probably be loaded in a manifest file or something.
		//const std::string textures[] = {
		//	"assets/player.png",
		//	"assets/fire.png",
		//	"assets/tree1.png",
		//	"assets/tree2.png",
		//	"assets/tree3.png",
		//	"assets/test.png",
		//	"assets/test_tree.png",
		//	"assets/large_sprite_test.png",
		//	"assets/para_test_0.png",
		//	"assets/para_test_1.png",
		//	"assets/para_test_2.png",
		//	"assets/para_test_outline.png",
		//};
		//const std::string shaders[] = {
		//	"shaders/box2d_debug",
		//	"shaders/terrain",
		//	"shaders/sprite",
		//	"shaders/parallax",
		//	"shaders/gui_poly",
		//	"shaders/gui_glyph",
		//	"shaders/fullscreen_passthrough",
		//};
		// TODO: with our new loader/manager scheme we no longer have an add function. how should we handle this kind of stuff?
		//for (const auto& path : textures) { engine.getTextureManager().add(path); }
		//for (const auto& path : shaders) { engine.getShaderManager().add(path); }
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// World
	////////////////////////////////////////////////////////////////////////////////////////////////
	Game::World& world = engine.getWorld();
	auto& guiContext = engine.getUIContext();
	guiContext.setNativeWindowHandle(window.getWin32WindowHandle());
	windowCallbacks.userdata = &engine;

	// Known problematic tick discrepancy. Right on the edge of sequence number
	// wrapping. This case _should_ be handled correctly. Often useful for
	// finding bugs.
	//
	// Don't do this in release builds so that we have the maximum availible tick space.
	if constexpr (ENGINE_DEBUG) {
		world.setNextTick(ENGINE_SERVER ?  0x7FFF'FFFF : 0);
		//world.setNextTick((uint32)Engine::Noise::lcg(std::random_device()()));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialize Systems
	////////////////////////////////////////////////////////////////////////////////////////////////
	setupBinds(engine);
	setupCommands(engine, window);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Main Loop
	////////////////////////////////////////////////////////////////////////////////////////////////
	const auto& cfg = Engine::getGlobalConfig();
	std::array<float, 64> deltas = {};
	size_t deltaIndex = 0;

	if constexpr (ENGINE_SERVER) {
		window.setPosSize(3440, 0, 1920, 1080);
	} else {
		window.setClientArea(1920, 1080);
		window.setPosition(1250, 100);
		//window.center();
	}

	ENGINE_LOG("Swap Interval: ", cfg.cvars.r_vsync);
	window.setSwapInterval(cfg.cvars.r_vsync);
	window.show();

	{
		const auto endTime = Engine::Clock::now();
		ENGINE_INFO("Launch Time: ", Engine::Clock::Milliseconds{endTime - launchTime}.count(), "ms");
		ENGINE_INFO("Start Time: ", Engine::Clock::Milliseconds{endTime - startTime}.count(), "ms");
	}

	// Configure Windows clock resolution
	#if ENGINE_OS_WINDOWS
		const auto minClockResolution = []{
			TIMECAPS caps{ .wPeriodMin = 0 };
			::timeGetDevCaps(&caps, sizeof(caps));
			return std::chrono::milliseconds{caps.wPeriodMin};
		}();

		ENGINE_ASSERT(minClockResolution < std::chrono::milliseconds{100}); // Sanity check

		if (minClockResolution != std::chrono::milliseconds{1}) {
			ENGINE_WARN2("Poor cpu clock resolution ({}, expected 1ms). You may experience excessive hardware usage.", minClockResolution);
		}

		timeBeginPeriod(static_cast<UINT>(minClockResolution.count()));
	#endif

	glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
	while (!window.shouldClose()) {
		const auto tstart = std::chrono::high_resolution_clock::now();
		window.poll();

		// Rendering
		glClear(GL_COLOR_BUFFER_BIT);

		// ECS
		world.run();
		
		// Frame rate
		deltas[deltaIndex] = world.getDeltaTime();
		deltaIndex = ++deltaIndex % deltas.size();
		if (deltaIndex == 0) {
			avgDeltaTime = std::accumulate(deltas.cbegin(), deltas.cend(), 0.0) / deltas.size();
		}

		window.swapBuffers();

		// TODO: If gsync or adaptive sync is enabled:
		//       If enabled default r_vsync = 0
		//       If disabled or not supported: r_vsync = 1
		// TODO: Look into NV_CTRL_GSYNC_ALLOWED and NVAPI for gsync and EXT_swap_control_tear for Adaptive Sync
		// TODO: Look into NVIDIA Reflex SDK and AMD Anti-Lag
		// 
		// Limit our framerate if we are out of focus or have vsync disabled
		// Don't eat all our CPU/GPU and cause our system to prepare for takeoff.
		if (ENGINE_SERVER || !window.hasFocus() || cfg.cvars.r_vsync == 0) {
			const auto frametime = [&]() ENGINE_INLINE {
				if constexpr (ENGINE_CLIENT) {
					return std::chrono::milliseconds{static_cast<uint32>(window.hasFocus() ? cfg.cvars.r_frametime : cfg.cvars.r_frametime_bg)};
				} else {
					return world.getTickInterval(); // TODO: should probably be a cmdline/cvar
				}
			}();

			if (frametime > minClockResolution) {
				const auto target = tstart + (frametime - minClockResolution);
				std::this_thread::sleep_until(target);
			}
		}
	}

	#if ENGINE_OS_WINDOWS
		timeEndPeriod(static_cast<UINT>(minClockResolution.count()));
	#endif
}

static_assert(ENGINE_CLIENT ^ ENGINE_SERVER, "Must be either client or server");
int entry(int argc, char* argv[]) {
	launchTime = Engine::Clock::now();

	if (!Engine::Net::startup()) {
		ENGINE_ERROR("Unable to start networking: ", Engine::Win32::getLastErrorMessage());
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Command Line
	////////////////////////////////////////////////////////////////////////////////////////////////
	Engine::CommandLine::Parser parser;

	{
		using namespace Engine::CommandLine;
		using namespace Engine::Net;

		parser
			.add<uint16>("port", 'p', ENGINE_SERVER ? 21212 : 0,
				"The port to listen on.")
			.add<IPv4Address>("group", 'g', {224,0,0,212, 12121},
				"The multicast group to join for server discovery. Zero to disable.")
			.add<std::string>("log", 'l', "",
				"The file to use for logging.")
			.add<bool>("logColor",
				"Enable or disable color log output.")
			.add<bool>("logTimeOnly",
				"Show only time stamps instead of full dates in log output.")
		;

		parser.parse(argc - 1, argv + 1);
		auto& cfg = Engine::getGlobalConfig<true>();

		{ // Setup first part of global log config
			const auto* log = parser.get<std::string>("log");

			if (log && !log->empty()) {
				std::cout << "Log file: " << *log << std::endl;
				auto old = std::move(cfg.log);
				// TODO: cfg.log = {fopen(log->c_str(), "ab+"), &fclose};
				cfg.log = {fopen(log->c_str(), "ab+"), &fclose};

				if (!cfg.log) {
					cfg.log = std::move(old);
					ENGINE_WARN("Failed to open log file: ", *log);
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Configure Console
	////////////////////////////////////////////////////////////////////////////////////////////////
	if(!AllocConsole()) {
		ENGINE_WARN("Unable to allocate console window - ", Engine::Win32::getLastErrorMessage());
	} else {
		FILE* unused;
		freopen_s(&unused, "CONIN$", "r", stdin);
		freopen_s(&unused, "CONOUT$", "w", stdout);
		freopen_s(&unused, "CONOUT$", "w", stderr);
	}

	if (auto console = GetStdHandle(STD_OUTPUT_HANDLE);
		!console || !SetConsoleMode(console, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
		ENGINE_WARN(Engine::Win32::getLastErrorMessage());
	}

	// Position the console
	if (HWND window; window = GetConsoleWindow()) {
		constexpr int height = 1440/2;
		if constexpr (ENGINE_CLIENT) {
			SetWindowPos(window, HWND_TOP, 0, 0, 1500, height, 0);
			SetWindowTextW(window, L"Client");
		} else if constexpr (ENGINE_SERVER) {
			SetWindowPos(window, HWND_TOP, 0, height, 1500, height, 0);
			SetWindowTextW(window, L"Server");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Setup global config
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Win32: This must be after AllocConsole and SetConsoleMode or else isatty wont work
	{ 
		auto& cfg = Engine::getGlobalConfig<true>();

		{ // Setup second part of global log config
			const auto* logColor = parser.get<bool>("logColor");
			const auto* logTimeOnly = parser.get<bool>("logTimeOnly");
			const bool isTerminal = isatty(fileno(cfg.log.get()));
			cfg.logColor = logColor ? *logColor : isTerminal;
			cfg.logTimeOnly = logTimeOnly ? *logTimeOnly : isTerminal;
		}

		{ // Setup global network config
			const auto* port = parser.get<uint16>("port");
			if (port) { cfg.port = *port; }

			const auto* group = parser.get<Engine::Net::IPv4Address>("group");
			if (group) { cfg.group = *group; }
		}

		{ // Setup logger
			const auto writter = []<bool TimeOnly, bool Style>{
				return [](const Engine::Logger& logger, const Engine::Logger::Info& info, std::string_view format, fmt::format_args args){
					fmt::memory_buffer buffer;
					auto out = std::back_inserter(buffer);
					logger.decorate<TimeOnly, Style>(out, info);
					fmt::vformat_to(out, format, args);
					out = '\n';
					fwrite(buffer.data(), 1, buffer.size(), Engine::getGlobalConfig().log.get());
				};
			};

			if (cfg.logColor && cfg.logTimeOnly) {
				cfg.logger.styledWritter = writter.template operator()<true, true>();
			} else if (cfg.logColor) {
				cfg.logger.styledWritter = writter.template operator()<false, true>();
			} else if (cfg.logTimeOnly) {
				cfg.logger.styledWritter = writter.template operator()<true, false>();
			} else {
				cfg.logger.styledWritter = writter.template operator()<false, false>();
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Other
	////////////////////////////////////////////////////////////////////////////////////////////////
	ENGINE_INFO("Starting ", ENGINE_SERVER ? "Server" : "Client");
	ENGINE_INFO("Working Directory: ", std::filesystem::current_path().generic_string());

	signal(SIGABRT, [](int){ performExit("abort signal received"); });
	signal(SIGTERM, [](int){ performExit("terminate signal received"); });

	#ifdef ENGINE_OS_WINDOWS
		SetConsoleCtrlHandler([](DWORD ctrlType) -> BOOL {
			const char* type = nullptr;

			switch(ctrlType) {
				case CTRL_C_EVENT: { type = "CTRL_C_EVENT"; break; }
				case CTRL_BREAK_EVENT: { type = "CTRL_BREAK_EVENT"; break; }
				case CTRL_CLOSE_EVENT: { type = "CTRL_CLOSE_EVENT"; break; }
				case CTRL_LOGOFF_EVENT: { type = "CTRL_LOGOFF_EVENT"; break; }
				case CTRL_SHUTDOWN_EVENT: { type = "CTRL_SHUTDOWN_EVENT"; break; }
			}

			if (type) {
				std::string msg = "win32 control handler called with ";
				msg += type;
				performExit(msg.c_str());
			}

			return false;
		}, true);
	#endif

	std::atexit([]{ performExit("exit requested"); });
	std::at_quick_exit([]{ performExit("quick exit requested"); });
	std::set_terminate([]{ performExit("terminate handler called"); });

	{
		const auto& info = Engine::Win32::getStartupInfo();
		SetConsoleTitleW(fmt::format(L"{} - Console - {}", info.type, info.debugging).c_str());
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Run
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// At seemingly random the debugger decides to not work. Enable, run, disable, run seems to fix this for some reason.
	// Other times spam clicking on Visual Studio while the program launches fixes this. 10/10.
	// Minimizing the server window before crash seems to help
	// --------------------------
	// Function Breakpoints: assert, _wassert, abort, exit
	// --------------------------
	//_set_error_mode(_OUT_TO_MSGBOX);
	//_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW | _CRTDBG_MODE_DEBUG);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: move to gtest once fixed.
	test_divFloor();
	test_divCeil();
	test_AreaWalker();

	startTime = Engine::Clock::now();
	run(argc, argv);

	if (!Engine::Net::shutdown()) {
		ENGINE_WARN("Unable to shutdown networking: ", Engine::Win32::getLastErrorMessage());
	}

	ENGINE_LOG("Done.");
	return EXIT_SUCCESS;
}

#ifdef ENGINE_OS_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
	if (ENGINE_SERVER) {
		//std::this_thread::sleep_for(std::chrono::milliseconds{2'000});
	}

	return entry(__argc, __argv);
}
#else
int main(int argc, char* argv[]) {
	return entry(argc, argv);
}
#endif
