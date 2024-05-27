// Game
#include <Game/UI/TerrainPreview.hpp>
#include <Game/TerrainGenerator.hpp>

// Engine
#include <Engine/UI/ImageDisplay.hpp>
#include <Engine/UI/TextBox.hpp>


namespace {
	using namespace Game;
	using namespace Game::UI;

	struct BiomeOne {
		template<Game::Terrain::StageId Stage>
		static void stage(void* self, Game::Terrain::Terrain& terrain, const Game::ChunkVec chunkCoord, Game::Terrain::Chunk& chunk, const Game::Terrain::BiomeInfo biomeInfo) {
			static_assert(Stage != Stage, "foo bar");
		};

		template<>
		static void stage<1>(void* self, Game::Terrain::Terrain& terrain, const Game::ChunkVec chunkCoord, Game::Terrain::Chunk& chunk, const Game::Terrain::BiomeInfo biomeInfo) {
			for (uint32 x = 0; x < chunkSize.x; ++x) {
				for (uint32 y = 0; y < chunkSize.y; ++y) {
					if ((x & 1) ^ (y & 1)) {
						chunk.data[x][y] = BlockId::Debug;
					} else {
						chunk.data[x][y] = BlockId::Debug2;
					}
				}
			}
		}
	};

	struct BiomeTwo {
		template<Game::Terrain::StageId> static void stage(void* self, Game::Terrain::Terrain& terrain, const Game::ChunkVec chunkCoord, Game::Terrain::Chunk& chunk, const Game::Terrain::BiomeInfo biomeInfo) {};
	};

	struct BiomeThree {
		template<Game::Terrain::StageId> static void stage(void* self, Game::Terrain::Terrain& terrain, const Game::ChunkVec chunkCoord, Game::Terrain::Chunk& chunk, const Game::Terrain::BiomeInfo biomeInfo) {};
	};

	class TerrainDragArea : public EUI::ImageDisplay {
		public:
			glm::vec2 offset = {0.0, 0.0};
			glm::vec2 zoom = {1.0, 1.0};

		private:
			// View
			glm::vec2 move = {0, 0};
			float32 zoomAccum = 0;

			// Image
			Engine::Gfx::Image img = {};
			Engine::Gfx::Texture2D tex = {};

			// Terrain
			Terrain::Generator<1, BiomeOne, BiomeTwo, BiomeThree> generator{1234};
			Terrain::Terrain terrain;

		public:
			TerrainDragArea(EUI::Context* context)
				: ImageDisplay{context}
				, img{Engine::Gfx::PixelFormat::RGB8, {512, 512}} {

				img.fill({255, 255, 0});
				tex.setAuto(img);
				setTexture(tex);
			}

			void rebuild() {

				// (0,0) through (8,8) = 512px / (16 blocks / chunk) = 512 / 16 = 8
				generator.generate1(terrain, Terrain::Request{{0, 0}, {8, 8}, 0});

				// TODO: Move this color specification to Blocks.xpp, could be useful
				//       elsewhere. Alternatively, calculate this value based on the avg img
				//       color when loading them/packing them into atlas.
				glm::u8vec3 blockToColor[BlockId::_count] = {};
				blockToColor[BlockId::Entity]	= {0, 120, 189};
				blockToColor[BlockId::Debug]	= {255,   0,   0};
				blockToColor[BlockId::Debug2]	= {200,  26, 226};
				blockToColor[BlockId::Debug3]	= {226,  26, 162};
				blockToColor[BlockId::Debug4]	= {226,  26, 111};
				blockToColor[BlockId::Dirt]		= {158,  98,  33};
				blockToColor[BlockId::Grass]	= { 67, 226,  71};
				blockToColor[BlockId::Iron]		= {144, 144, 144};
				blockToColor[BlockId::Gold]		= {255, 235,  65};

				auto* data = reinterpret_cast<glm::u8vec3*>(img.data());

				const RealmId realmId = 0;
				const auto& res = img.size();
				for (BlockUnit y = 0; y < res.y; ++y) {
					const auto yspan = y * res.x;
					for (BlockUnit x = 0; x < res.x; ++x) {
						const auto blockCoord = UniversalBlockCoord{realmId, {x, y}};
						const auto chunkCoord = blockCoord.toChunk();
						const auto regionCoord = chunkCoord.toRegion();
						auto& region = terrain.getRegion(regionCoord);
						auto& chunk = region.chunks[chunkCoord.pos.x][chunkCoord.pos.y];
						ENGINE_DEBUG_ASSERT(region.stages[chunkCoord.pos.x][chunkCoord.pos.y] == 1);

						const auto chunkIndex = blockCoord.pos - chunkCoord.toBlock().pos;
						ENGINE_DEBUG_ASSERT(chunkIndex.x >= 0 && chunkIndex.x < chunkSize.x);
						ENGINE_DEBUG_ASSERT(chunkIndex.y >= 0 && chunkIndex.y < chunkSize.y);

						const auto blockId = chunk.data[chunkIndex.x][chunkIndex.y];
						ENGINE_DEBUG_ASSERT(blockId >= 0 && blockId < BlockId::_count);

						data[x + yspan] = blockToColor[blockId];
					}
				}

				// TODO: why does fixed size not work here. Fixed size should always work.
				setFixedSize(img.size()); // TODO: rm - just for testing

				tex.setImage(img);
			};
			
			glm::vec2 scale() const noexcept { return getSize() / glm::vec2{img.size()}; }

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
}

namespace Game::UI {
	TerrainPreview::TerrainPreview(EUI::Context* context) : Window{context} {
		const auto& theme = ctx->getTheme();
		const auto cont = getContent();
		
		const auto area = ctx->constructPanel<TerrainDragArea>();

		const auto sec = ctx->createPanel<Panel>(cont);
		sec->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, theme.sizes.pad1});
		sec->setAutoSizeHeight(true);
		
		constexpr auto textGetter = [](auto& var){ return [&var, last=0.0f](EUI::TextBox& box) mutable {
			if (var == last) { return; }
			last = var;
			box.setText(std::to_string(last));
		};};
		
		const auto textSetter = [area](auto& var){ return [area, &var](EUI::TextBox& box) {
			std::from_chars(std::to_address(box.getText().begin()), std::to_address(box.getText().end()), var);
			area->rebuild();
		};};
		
		const auto xMove = ctx->createPanel<EUI::TextBox>(sec);
		xMove->autoSize();
		xMove->bind(textGetter(area->offset.x), textSetter(area->offset.x));
		
		const auto yMove = ctx->createPanel<EUI::TextBox>(sec);
		yMove->autoSize();
		yMove->bind(textGetter(area->offset.y), textSetter(area->offset.y));
		
		const auto xZoom = ctx->createPanel<EUI::TextBox>(sec);
		xZoom->autoSize();
		xZoom->bind(textGetter(area->zoom.x), textSetter(area->zoom.x));
		
		const auto yZoom = ctx->createPanel<EUI::TextBox>(sec);
		yZoom->autoSize();
		yZoom->bind(textGetter(area->zoom.y), textSetter(area->zoom.y));

		// TODO: we really should have fixedSize = true/false and then just use the actual
		//       size for the size, that would allow autoSize to still work with a fixed size
		//       (i.e. no layout resizing).
		sec->setFixedHeight(sec->getHeight());

		cont->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Stretch, EUI::Align::Stretch, theme.sizes.pad1});
		cont->addChild(area);

		// TODO: probably just tie this to a bind like we do for zoom panel and re-enable.
		//setCloseCallback([](EUI::Window* win){ win->getContext()->deferredDeletePanel(win); });
	}
}
