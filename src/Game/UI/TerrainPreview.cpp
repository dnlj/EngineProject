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
		template<Terrain::StageId Stage>\
		BlockId stage(Terrain::Terrain& terrain, const ChunkVec chunkCoord, const BlockVec blockCoord, Terrain::Chunk& chunk, const Terrain::BiomeInfo biomeInfo) {\
			static_assert(Stage != Stage, "The requested stage is not defined for this biome.");\
		}

	#define STAGE(N) template<> ENGINE_INLINE_REL BlockId stage<N>(Terrain::Terrain& terrain, const ChunkVec chunkCoord, const BlockVec blockCoord, Terrain::Chunk& chunk, const Terrain::BiomeInfo biomeInfo)

	// TODO: store this info somewhere, we can't use this for biomes because it will break once we have non-independant generation.
	/*
	ENINGE_RUNTIME_CHECKS_DISABLE struct RescaleBiome {
		ENGINE_INLINE_REL std::tuple<int64, int64> rescale(const int64 x, const int64 y) const noexcept {
			// NOTE: These numbers here are still correct (relatively speaking), but at the
			//       time this was measure we were accidentally doing generation
			//       per-chunk/per-chunk. Meaning that we were generating a entire chunk per
			//       block, or 64^2 extra generations. We were also operating on int32 and
			//       double instead of int64 and double, so think might be slightly different.

			// This is unfortunately needed because doing the simple thing of using
			// std/glm::round is unusably slow. SSE is over an order of magnitude faster:
			// 6s vs 62s+ in debug and 2s vs 20s in release.
			if constexpr (true) {
				// TODO: use a lib, this is all SSE2
				// TODO: may need _MM_SET_ROUNDING_MODE
				const auto xy = _mm_set_pd(static_cast<double>(y+trans.y), static_cast<double>(x+trans.x));
				const auto factor = _mm_set_pd1(scale);
				const auto scaled = _mm_mul_pd(xy, factor);
				const auto rounded = _mm_round_pd(scaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
				return {(int64)rounded.m128d_f64[0], (int64)rounded.m128d_f64[1]};

				// Direct access (MSVC specific) ~10.1s
				// = no additional instructions + no stack checking
				//return {rounded.m128i_i32[0], rounded.m128i_i32[1]};

				// memcpy ~25.5s
				// = 1x mov, 2x lea, 1x call memcpy + stack checking
				//alignas(16) int32 results[4];
				//memcpy(results, &rounded, sizeof(rounded));
				//return {results[0], results[1]};

				// mm_store ~25.5s
				// = 2x movdqa + stack checking = same as union
				//alignas(16) int32 results[4];
				//_mm_store_si128((__m128i*)results, rounded);
				//return {results[0], results[1]};

				// Union ~25.5s
				// = 2x movdqa + stack checking = same as mm_store
				//union alignas(16) DataU {
				//	__m128i sse;
				//	alignas(16) int32 arr[4];
				//} data = rounded;
				//return {data.arr[0], data.arr[1]};

				// Union (direct) ~25.1s
				// = no additional instructions + stack checking = same as direct w/ additional stack checking
				// With RTC checks disasbled this is _almost_ as fast as the direct, the
				// stack check calls are still there, but I assume they don't actually do
				// anything. With them disabled its ~10.5s.
				// TODO: See if this can be replaced with start_lifetime_as_array once
				//       MSVC implements it so it isn't UB (strict aliasing). MSVC use to
				//       explicitly allow uninos to alias, but I'm not able to find that
				//       citation anymore. I'm not sure if they removed that or I'm just blind.
				//union alignas(16) DataU {
				//	__m128i sse;
				//	alignas(16) int32 arr[4];
				//} data = {_mm_cvtpd_epi32(scaled)};
				//return {data.arr[0], data.arr[1]};

				// bit_cast ~28.9s
				// = 2x movaps, 3x movdqa, 1x movups, 1x lea, 2x mov + stack checking
				//struct alignas(16) DataB {
				//	alignas(16) int32 results[4];
				//} data = std::bit_cast<DataB>(rounded);
				//return {data.results[0], data.results[1]};

				// bit_cast (direct) ~28.9s
				// exact same as non-direct
				//struct alignas(16) DataB {
				//	alignas(16) int32 results[4];
				//} data = std::bit_cast<DataB>(_mm_cvtpd_epi32(scaled));
				//return {data.results[0], data.results[1]};
			} else {
				ENGINE_FLATTEN
				return {
					static_cast<int32>(glm::round(x * scale)),
					static_cast<int32>(glm::round(y * scale)),
				};
			}
		}
	} ENINGE_RUNTIME_CHECKS_RESTORE;
	*/

	struct BiomeOne {
		STAGE_DEF;

		Engine::Noise::OpenSimplexNoise noise{1234};

		STAGE(1) {
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug2; }
			constexpr float32 scale = 0.1f;
			return noise.value(blockCoord.x * scale, blockCoord.y * scale) > 0 ? BlockId::Debug : BlockId::Air;
			//if ((x & 1) ^ (y & 1)) {
			//	return BlockId::Debug;
			//} else {
			//	return BlockId::Air;
			//}
		}
	};

	struct BiomeTwo {
		STAGE_DEF;

		STAGE(1) {
			if (blockCoord.x == blockCoord.y) {
				return BlockId::Gold;
			} else {
				return BlockId::Dirt;
			}
		}
	};

	struct BiomeThree {
		STAGE_DEF;

		STAGE(1) {
			if (blockCoord.x == 63 - blockCoord.y) {
				return BlockId::Grass;
			} else {
				return BlockId::Air;
			}
		}
	};

	class TerrainDragArea : public EUI::ImageDisplay {
		public:
			//BlockVec offset = {0.0, 0.0};
			BlockVec offset = {-127, -69};
			float64 zoom = 1.0f; // Larger # = farther out = see more = larger FoV

		private:
			// View
			glm::vec2 offsetInitial = {0, 0};
			Engine::Clock::TimePoint nextZoom{};

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
						ENGINE_DEBUG_ASSERT(region.stages[chunkIndex.x][chunkIndex.y] == 1, "Chunk is at incorrect stage.");

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

		// TODO: probably just tie this to a bind like we do for zoom panel and re-enable.
		//setCloseCallback([](EUI::Window* win){ win->getContext()->deferredDeletePanel(win); });
	}
}
