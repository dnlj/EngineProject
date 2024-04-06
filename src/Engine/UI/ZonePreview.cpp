// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>
#include <Game/UI/ZonePreview.hpp>

// Engine
#include <Engine/UI/ImageDisplay.hpp>
#include <Engine/Math/color.hpp>

namespace Game::UI { namespace {
	class ZoneDragArea : public EUI::ImageDisplay {
		public: // TODO: private
			glm::vec2 move = {};
			float32 zoomAccum = 0;
			Engine::Clock::TimePoint lastZoom = {};
			Engine::Gfx::Image img = {};
			Engine::Gfx::Texture2D tex = {};
			glm::vec2 offset = {0.0, 0.0};
			glm::vec2 zoom = {0.5, 0.5};

			std::vector<glm::u8vec3> zoneColors;

			Engine::FlatHashSet<ChunkVec> lastActiveChunks;
			Engine::Clock::TimePoint lastUpdate;

			// Arbitrary starting color. Saturation and lightness will be
			// maintained between all generated colors.
			glm::vec3 nextColorHSL = {319, 0.85, 0.50};

		public:
			ZoneDragArea(EUI::Context* context)
				: ImageDisplay{context}
				, img{Engine::Gfx::PixelFormat::RGB8, {512, 512}} {

				img.fill({255, 255, 0});
				tex.setAuto(img);
				setTexture(tex);
			}

			void render() override {
				auto* engine = ctx->getUserdata<Game::EngineInstance>();
				auto& world = engine->getWorld();

				if (world.getTickTime() - lastUpdate > std::chrono::milliseconds{500}) {
					lastUpdate = world.getTickTime();

					const auto& mapSys = world.getSystem<MapSystem>();
					const auto& activeChunks = mapSys.getActiveChunks();
					const auto tick = world.getTick();
					bool shouldRebuild = false;

					// A chunk was unloaded
					const auto activeChunksEnd = activeChunks.end();
					for (auto it = lastActiveChunks.begin(); it != lastActiveChunks.end();) {
						const auto found = activeChunks.find(*it);

						if (found == activeChunksEnd) {
							shouldRebuild = true;
							it = lastActiveChunks.erase(it);
						} else {
							++it;
						}
					}

					// A chunk has been loaded or updated
					for (const auto& [pos, chunk] : activeChunks) {
						lastActiveChunks.emplace(pos);
						shouldRebuild = shouldRebuild || chunk.updated == tick;
					}

					if (shouldRebuild) {
						rebuild();
					}
				}

				ImageDisplay::render();

				if (zoomAccum && (Engine::Clock::now() - lastZoom) > std::chrono::milliseconds{1000}) {
					lastZoom = Engine::Clock::now();
					rebuild();
				}
			}

			glm::vec2 scale() { return getSize() / glm::vec2{img.size()}; }

			void rebuild() {
				// TODO: pan/zoom
				//if (zoomAccum) {
				//	auto z = std::clamp(zoomAccum * 0.2f, -0.9f, 0.9f);
				//	zoom -= zoom * z;
				//	zoom = glm::max(zoom, 0.05f);
				//	zoomAccum = 0;
				//}
				//mapTest(offset.x, offset.y, zoom.x, zoom.y);

				auto* engine = ctx->getUserdata<Game::EngineInstance>();
				auto& world = engine->getWorld();
				const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
				const auto& mapSys = world.getSystem<MapSystem>();
				const auto& activeChunks = mapSys.getActiveChunks();
				const auto& regions = mapSys.getLoadedRegions();

				// TODO: Automatically rescale based on min/max
				//ChunkVec min{std::numeric_limits<ChunkUnit>::min(), std::numeric_limits<ChunkUnit>::min()};
				//ChunkVec max{std::numeric_limits<ChunkUnit>::max(), std::numeric_limits<ChunkUnit>::max()};
				//for (const auto& [pos, _] : activeChunks) {
				//	min = glm::min(min, pos);
				//	max = glm::max(max, pos);
				//}

				while (zoneSys.getZones().size() > zoneColors.size()) {
					zoneColors.push_back(glm::u8vec3{255.0f * Engine::Math::cvtHSLtoRGB(nextColorHSL)});
					nextColorHSL.x = Engine::Math::nextRandomHue(nextColorHSL.x);
				}

				ENGINE_DEBUG_ASSERT(img.format() == Engine::Gfx::PixelFormat::RGB8, "This code assumes rgb (8, 8, 8) format.");
				const auto& res = img.size();
				byte* data = img.data();
				for (int32 y = 0; y < res.y; ++y) {
					for (int32 x = 0; x < res.x; ++x) {
						// Offset by 256 so (0, 0) is centered instead of bottom left.
						const ChunkVec chunkPos = {x - 256, y - 256};
						glm::u8vec3 color = {255, 255, 0};

						const auto& chunkData = activeChunks.find(chunkPos);

						if (chunkData != activeChunks.end()) {
							const auto zoneId = chunkData->second.body.getZoneId();
							color = zoneColors[zoneId];
						} else if (regions.contains(chunkToRegion(chunkPos))) {
							color = {70, 70, 70};
						} else {
							color = {50, 50, 50};
						}

						data[3 * (y * res.x + x) + 0] = color.r;
						data[3 * (y * res.x + x) + 1] = color.g;
						data[3 * (y * res.x + x) + 2] = color.b;
					}
				}

				// Fix size so 1chunk=1pixel
				setFixedSize(img.size());
				tex.setImage(img);
			}

			bool onAction(EUI::ActionEvent action) override {
				switch (action) {
					case EUI::Action::Scroll: {
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
				move -= ctx->getCursor();
				if (move.x || move.y) {
					move.y = -move.y;
					const glm::ivec2 diff = glm::round(move * zoom / scale());
					offset += diff;
					rebuild();
				}
			}
	};
}}

namespace Game::UI {
	ZonePreview::ZonePreview(EUI::Context* context) : Window{context} {
		const auto& theme = ctx->getTheme();
		const auto cont = getContent();
		
		const auto area = ctx->constructPanel<ZoneDragArea>();

		// TODO: enable once zoom / pan works
		//const auto sec = ctx->createPanel<Panel>(cont);
		//sec->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, theme.sizes.pad1});
		//sec->setAutoSizeHeight(true);
		//
		//constexpr auto textGetter = [](auto& var){ return [&var, last=0.0f](EUI::TextBox& box) mutable {
		//	if (var == last) { return; }
		//	last = var;
		//	box.setText(std::to_string(last));
		//};};
		//
		//const auto textSetter = [area](auto& var){ return [area, &var](EUI::TextBox& box) {
		//	std::from_chars(std::to_address(box.getText().begin()), std::to_address(box.getText().end()), var);
		//	area->rebuild();
		//};};
		//
		//const auto xMove = ctx->createPanel<EUI::TextBox>(sec);
		//xMove->autoSize();
		//xMove->bind(textGetter(area->offset.x), textSetter(area->offset.x));
		//
		//const auto yMove = ctx->createPanel<EUI::TextBox>(sec);
		//yMove->autoSize();
		//yMove->bind(textGetter(area->offset.y), textSetter(area->offset.y));
		//
		//const auto xZoom = ctx->createPanel<EUI::TextBox>(sec);
		//xZoom->autoSize();
		//xZoom->bind(textGetter(area->zoom.x), textSetter(area->zoom.x));
		//
		//const auto yZoom = ctx->createPanel<EUI::TextBox>(sec);
		//yZoom->autoSize();
		//yZoom->bind(textGetter(area->zoom.y), textSetter(area->zoom.y));
		//
		//sec->setFixedHeight(sec->getHeight());

		cont->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Stretch, EUI::Align::Stretch, theme.sizes.pad1});
		cont->addChild(area);
		setSize({512, 512});
		setCloseCallback([](EUI::Window* win){ win->getContext()->deferredDeletePanel(win); });
	}
}
