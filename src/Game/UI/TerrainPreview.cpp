// Game
#include <Game/UI/TerrainPreview.hpp>
#include <Game/TerrainGenerator.hpp>

// Engine
#include <Engine/UI/ImageDisplay.hpp>
#include <Engine/UI/TextBox.hpp>

#include <Engine/Noise/SimplexNoise.hpp>
#include <Engine/tuple.hpp>


namespace {
	using namespace Game;
	using namespace Game::UI;

	#define STAGE_DEF \
		template<Terrain::StageId Stage> constexpr static bool hasStage = false;\
		template<Terrain::StageId Stage>\
		ENGINE_INLINE BlockId stage(TERRAIN_STAGE_ARGS) {\
			static_assert(Stage != Stage, "The requested stage is not defined for this biome.");\
		}

	#define STAGE(N)\
		template<> constexpr static bool hasStage<N> = true;\
		template<> ENGINE_INLINE_REL BlockId stage<N>(TERRAIN_STAGE_ARGS)

	using Float = float32;
	ENGINE_INLINE consteval Float operator""_f(const long double v) noexcept {
		return static_cast<Float>(v);
	}

	ENGINE_INLINE consteval Float operator""_f(const uint64 v) noexcept {
		return static_cast<Float>(v);
	}

	struct BiomeOne {
		STAGE_DEF;

		Engine::Noise::OpenSimplexNoise simplex{1234};

		STAGE(1) {
			// TODO: if we are always going to be converting to float anyways, should we
			//       pass in a float version as well? That kind of breaks world size though.

			const auto h1 = h0 + 15 * simplex.value(blockCoord.x * 0.05_f, 0); // TODO: 1d simplex

			if (blockCoord.y > h1) {
				return BlockId::Air;
			} else if ((h1-blockCoord.y) < 1) {
				return BlockId::Grass;
			}

			constexpr Float scale = 0.06_f;
			constexpr Float groundScale = 1.0_f / 100.0_f;
			const Float groundGrad = std::max(0.0_f, 1.0_f - (h1 - blockCoord.y) * groundScale);
			const auto val = simplex.value(glm::vec2{blockCoord} * scale) + groundGrad;

			if (val > 0) {
				return BlockId::Debug;
			} else {
				return BlockId::Air;
			}
		}

		//
		//
		//
		// TODO: non-square biomes
		//
		//
		//

		//STAGE(2) {
		//	auto& blockId = chunk.data[blockIndex.x][blockIndex.y];
		//	if (((blockCoord.x & 1) ^ (blockCoord.y & 1)) && (blockId == BlockId::Air)) {
		//		return BlockId::Debug2;
		//	} else {
		//		return blockId;
		//	}
		//}

		//
		//
		//
		// TODO: IS BIOME SIZE == CHUNK SIZE? probably not, i would guess that might be the "issue" (its not an issue) that i am seeing.
		// - It does not, and that is indeed the issue. Need to sample biome in the get function to ensure it is correct.
		// - **Instead maybe we should pass in block coords? Would probably make more sense.**
		//
		//
		//

		//
		//
		// TODO: Biome culling, sample corners and do the thing
		//
		//
		void getLandmarks(TERRAIN_GET_LANDMARKS_ARGS) {
			ENGINE_LOG2("GET LANDMARK: {}", chunkCoord);
			auto blockCoord = chunkToBlock(chunkCoord);
			inserter = {.min = blockCoord, .max = blockCoord, .id = 1};

			const auto maxX = blockCoord.x + chunkSize.x;
			for (; blockCoord.x < maxX; ++blockCoord.x) {
				if (blockCoord.x % 7 == 0) {
					inserter = {blockCoord, blockCoord + BlockVec{2, 8}, 0};
				}
			}
		}

		void genLandmarks(TERRAIN_GEN_LANDMARKS_ARGS) {
			// TODO: This is the least efficient way possible to do this. We need a good
			//       api to efficiently edit multiple blocks spanning multiple chunks and
			//       regions. We would need to pre split between both regions and then chunks
			//       and then do something roughly like:
			//       for region in splitRegions:
			//           for chunk in splitRegionChunks:
			//               applyEdit(chunk, editsForChunk(chunk));
			for (auto blockCoord = info.min; blockCoord.x <= info.max.x; ++blockCoord.x) {
				for (blockCoord.y = info.min.y; blockCoord.y <= info.max.y; ++blockCoord.y) {
					const auto chunkCoord = blockToChunk(blockCoord);
					const UniversalRegionCoord regionCoord = {info.realmId, chunkToRegion(chunkCoord)};
					auto& region = terrain.getRegion(regionCoord);
					const auto regionIdx = chunkToRegionIndex(chunkCoord);
					auto& chunk = region.chunks[regionIdx.x][regionIdx.y];
					const auto chunkIdx = blockToChunkIndex(blockCoord, chunkCoord);
					ENGINE_DEBUG_ASSERT(chunkIdx.x >= 0 && chunkIdx.x < chunkSize.x);
					ENGINE_DEBUG_ASSERT(chunkIdx.y >= 0 && chunkIdx.y < chunkSize.y);

					if (chunk.data[chunkIdx.x][chunkIdx.y] != BlockId::Debug4)
					{
						chunk.data[chunkIdx.x][chunkIdx.y] = info.id == 0 ? BlockId::Gold : BlockId::Debug3;
					}
					ENGINE_LOG2("GEN LANDMARK: {}", chunkCoord);
				}
			}
		}
	};

	struct BiomeDebugOne {
		STAGE_DEF;

		STAGE(1) {
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug2; }
			if ((blockCoord.x & 1) ^ (blockCoord.y & 1)) {
				return BlockId::Debug;
			} else {
				return BlockId::Air;
			}
		}
	};

	struct BiomeDebugTwo {
		STAGE_DEF;

		STAGE(1) {
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug3; }
			if (blockCoord.x % 64 == blockCoord.y % 64) {
				return BlockId::Gold;
			} else {
				return BlockId::Dirt;
			}
		}
	};

	struct BiomeDebugThree {
		STAGE_DEF;

		STAGE(1) {
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug4; }
			if (blockCoord.x % 64 == (63 - blockCoord.y % 64)) {
				return BlockId::Grass;
			} else {
				return BlockId::Air;
			}
		}
	};

	class TerrainDragArea : public EUI::ImageDisplay {
		public:
			BlockVec offset = {0.0, 0.0};
			//BlockVec offset = {-127, -69};
			float64 zoom = 1.0f; // Larger # = farther out = see more = larger FoV

		private:
			// View
			glm::vec2 offsetInitial = {0, 0};
			Engine::Clock::TimePoint nextZoom{};

			// Image
			Engine::Gfx::Image img = {};
			Engine::Gfx::Texture2D tex = {};

			// Terrain
			Terrain::Generator<BiomeOne, BiomeDebugTwo, BiomeDebugThree> generator{1234};
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
				// Chunk offset isn't affected by zoom. Its a fixed offset so its always the same regardless of zoom.
				const auto chunkOffset = blockToChunk(offset);
				const auto res = img.size();

				// We can't use blockToChunk because we need ceil not floor.
				// Say we have a res = 512, zoom = 1.2, blocksPerChunk = 64: 512 * 1.2 = 614.4; 614.4 / 64 = 9.6
				// For blocks we would say that that block lives in the chunk floor(9.6) = 9, but this isn't blocks.
				// This is how many chunks will fit in the resolution. The res will hold 9.6 chunks so we need to generate ceil(9.6) = 10.
				const auto chunksPerImg = Engine::Math::divCeil(applyZoom<BlockVec>(res), blocksPerChunk).q;

				terrain = {};
				std::atomic_signal_fence(std::memory_order_acq_rel); const auto startTime = Engine::Clock::now(); std::atomic_signal_fence(std::memory_order_acq_rel);
				generator.generate1(terrain, Terrain::Request{chunkOffset, chunkOffset + chunksPerImg, 0});
				std::atomic_signal_fence(std::memory_order_acq_rel); const auto endTime = Engine::Clock::now(); std::atomic_signal_fence(std::memory_order_acq_rel);
				ENGINE_LOG2("Generation Time: {}", Engine::Clock::Seconds{endTime - startTime});

				// TODO: Move this color specification to Blocks.xpp, could be useful
				//       elsewhere. Alternatively, calculate this value based on the avg img
				//       color when loading them/packing them into atlas.
				glm::u8vec3 blockToColor[BlockId::_count] = {};
				blockToColor[BlockId::Entity] = {  0, 120, 189};
				blockToColor[BlockId::Debug]  = {255,   0,   0};
				blockToColor[BlockId::Debug2] = {200,  26, 226};
				blockToColor[BlockId::Debug3] = {226,  26, 162};
				blockToColor[BlockId::Debug4] = {226,  26, 111};
				blockToColor[BlockId::Dirt]	  = {158,  98,  33};
				blockToColor[BlockId::Grass]  = { 67, 226,  71};
				blockToColor[BlockId::Iron]	  = {144, 144, 144};
				blockToColor[BlockId::Gold]	  = {255, 235,  65};

				auto* data = reinterpret_cast<glm::u8vec3*>(img.data());

				const RealmId realmId = 0; // TODO: realm support
				for (BlockUnit y = 0; y < res.y; ++y) {
					const auto yspan = y * res.x;
					for (BlockUnit x = 0; x < res.x; ++x) {
						const auto blockCoord = offset + applyZoom(BlockVec{x, y});
						const auto chunkCoord = blockToChunk(blockCoord);
						const auto regionCoord = chunkToRegion(chunkCoord);
						auto& region = terrain.getRegion({realmId, regionCoord});

						// Sanity checks
						ENGINE_DEBUG_ASSERT(blockCoord.x >= offset.x);
						ENGINE_DEBUG_ASSERT(blockCoord.y >= offset.y);

						// TODO: cant we just do chunkCoord - regionCoord.toChunk() which should be a lot cheaper?
						const auto chunkIndex = chunkToRegionIndex(chunkCoord, regionCoord);
						auto& chunk = region.chunks[chunkIndex.x][chunkIndex.y];
						const auto stage = region.stages[chunkIndex.x][chunkIndex.y];
						ENGINE_DEBUG_ASSERT(stage == generator.totalStages, "Chunk is at incorrect stage.");

						const auto blockIndex = blockToChunkIndex(blockCoord, chunkCoord);
						ENGINE_DEBUG_ASSERT(blockIndex.x >= 0 && blockIndex.x < chunkSize.x, "Invalid chunk index.");
						ENGINE_DEBUG_ASSERT(blockIndex.y >= 0 && blockIndex.y < chunkSize.y, "Invalid chunk index.");

						const auto blockId = chunk.data[blockIndex.x][blockIndex.y];
						ENGINE_DEBUG_ASSERT(blockId >= 0 && blockId < BlockId::_count, "Invalid BlockId.");

						data[x + yspan] = blockToColor[blockId];
					}
				}

				// TODO: why does fixed size not work here. Fixed size should always work.
				setFixedSize(img.size()); // TODO: rm - just for testing

				tex.setImage(img);
			};

			void render() override {
				ImageDisplay::render();
				if (Engine::Clock::now() > nextZoom) {
					ENGINE_INFO2("Zoom: {}", zoom);
					nextZoom = Engine::Clock::TimePoint::max();
					rebuild();
				}
			}

			// TODO: custom render for timeout, maybe just use an update func?

			bool onAction(EUI::ActionEvent action) override {
				switch (action) {
					case EUI::Action::Scroll: {
						ENGINE_LOG2("action.value.f32: {}", action.value.f32);
						zoom -= action.value.f32 * 0.2;
						nextZoom = Engine::Clock::now() + std::chrono::milliseconds{1000};
						return true;
					}
				}
				return false;
			}

			bool onBeginActivate() override {
				offsetInitial = ctx->getCursor();
				return true;
			}

			void onEndActivate() override {
				offsetInitial -= ctx->getCursor();
				if (offsetInitial.x || offsetInitial.y) {
					offsetInitial.y = -offsetInitial.y;
					const BlockVec diff = applyZoom(offsetInitial);
					offset += diff;
					rebuild();
				}
			}

		private:
			template<class Vec>
			constexpr Vec applyZoom(Vec v) noexcept { return glm::ceil(glm::f64vec2{v} * zoom); }
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

		constexpr auto textGetter = []<class V>(V& var){ return [&var, last=V{}](EUI::TextBox& box) mutable {
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

		//const auto xZoom = ctx->createPanel<EUI::TextBox>(sec);
		//xZoom->autoSize();
		//xZoom->bind(textGetter(area->zoom.x), textSetter(area->zoom.x));
		//
		//const auto yZoom = ctx->createPanel<EUI::TextBox>(sec);
		//yZoom->autoSize();
		//yZoom->bind(textGetter(area->zoom.y), textSetter(area->zoom.y));

		// TODO: we really should have fixedSize = true/false and then just use the actual
		//       size for the size, that would allow autoSize to still work with a fixed size
		//       (i.e. no layout resizing).
		sec->setFixedHeight(sec->getHeight());

		cont->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Stretch, EUI::Align::Stretch, theme.sizes.pad1});
		cont->addChild(area);

		ENGINE_DEBUG2("Area1: {}, Cont: {}", area->getSize(), cont->getSize());
		ENGINE_DEBUG2("Area2: {}, Cont: {}", area->getAutoHeight(), cont->getAutoHeight());

		// TODO: probably just tie this to a bind like we do for zoom panel and re-enable.
		//setCloseCallback([](EUI::Window* win){ win->getContext()->deferredDeletePanel(win); });
	}
}
