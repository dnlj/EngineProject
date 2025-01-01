// Game
#include <Game/UI/TerrainPreview.hpp>
#include <Game/TerrainGenerator.hpp>

// Engine
#include <Engine/UI/ImageDisplay.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/UI/Slider.hpp>

#include <Engine/Noise/SimplexNoise.hpp>
#include <Engine/tuple.hpp>
#include <Engine/Math/color.hpp>


namespace {
	using namespace Game;
	using namespace Game::UI;

	enum class Layer {
		BiomeRawWeights,
		BiomeBlendWeights,
		BiomeFinalWeights,
		BiomeFinalWeightsFull,
		Blocks,
		_count,
		_last = _count - 1,
	};
	ENGINE_BUILD_DECAY_ENUM(Layer);

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

		//STAGE(2) {
		//	auto& blockId = chunk.data[blockIndex.x][blockIndex.y];
		//	if (((blockCoord.x & 1) ^ (blockCoord.y & 1)) && (blockId == BlockId::Air)) {
		//		return BlockId::Debug2;
		//	} else {
		//		return blockId;
		//	}
		//}

		Float getBasisStrength(TERRAIN_GET_BASIS_ARGS) {
			return simplex.value(blockCoord);
		}

		void getLandmarks(TERRAIN_GET_LANDMARKS_ARGS) {
			//ENGINE_LOG2("GET LANDMARK: {}", chunkCoord);
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
					//ENGINE_LOG2("GEN LANDMARK: {}", chunkCoord);
				}
			}
		}
	};

	template<uint64 Seed>
	struct BiomeDebugBase {
		Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
		Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
		Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};

		Float getBasisStrength(TERRAIN_GET_BASIS_ARGS) {
			// These need to be tuned based on biome scales blend dist or else you can get odd clipping type issues.
			return 0.2f * simplex1.value(glm::vec2{blockCoord} * 0.003f)
				 + 0.2f * simplex2.value(glm::vec2{blockCoord} * 0.010f)
				 + 0.1f * simplex3.value(glm::vec2{blockCoord} * 0.100f)
				 + 0.5f;
		}
	};

	struct BiomeDebugOne : public BiomeDebugBase<0xF7F7'F7F7'F7F7'1111> {
		STAGE_DEF;
		STAGE(1) {
			return BlockId::Debug;
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug2; }
			if ((blockCoord.x & 1) ^ (blockCoord.y & 1)) {
				return BlockId::Debug;
			} else {
				return BlockId::Air;
			}
		}
		
		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			return std::abs(simplex1.value(glm::vec2{blockCoord} * 0.03_f)) - 0.15_f;
		}
	};
	
	struct BiomeDebugTwo : public BiomeDebugBase<0xF7F7'F7F7'F7F7'2222> {
		STAGE_DEF;
		STAGE(1) {
			return BlockId::Debug2;
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug3; }
			if (blockCoord.x % 64 == blockCoord.y % 64) {
				return BlockId::Gold;
			} else {
				return BlockId::Dirt;
			}
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			return std::abs(simplex1.value(glm::vec2{blockCoord} * 0.06_f)) - 0.75_f;
		}
	};
	
	struct BiomeDebugThree : public BiomeDebugBase<0xF7F7'F7F7'F7F7'3333> {
		STAGE_DEF;
		STAGE(1) {
			return BlockId::Debug3;
			if (blockCoord.x < 0 || blockCoord.y < 0) { return BlockId::Debug4; }
			if (blockCoord.x % 64 == (63 - blockCoord.y % 64)) {
				return BlockId::Grass;
			} else {
				return BlockId::Air;
			}
		}

		Float getBasis(TERRAIN_GET_BASIS_ARGS) {
			return simplex1.value(glm::vec2{blockCoord} * 0.12_f);
		}
	};

	class TerrainDragArea : public EUI::ImageDisplay {
		public:
			BlockVec offset = {0.0, 0.0};
			//BlockVec offset = {-127, -69};
			float64 zoom = 9.5f; // Larger # = farther out = see more = larger FoV
			Layer mode = Layer::BiomeFinalWeights;

		private:
			// View
			glm::vec2 offsetInitial = {0, 0};
			Engine::Clock::TimePoint nextRebuild{};
			constexpr static inline Engine::Clock::Duration rebuildDelay = std::chrono::milliseconds{300};

			// Image
			Engine::Gfx::Image img = {};
			Engine::Gfx::Texture2D tex = {};

			// Terrain
			//Terrain::Generator<BiomeOne, BiomeDebugTwo, BiomeDebugThree> generator{1234};
			Terrain::Generator<BiomeDebugOne, BiomeDebugTwo, BiomeDebugThree> generator{1234};
			Terrain::Terrain terrain;

		public:
			TerrainDragArea(EUI::Context* context)
				: ImageDisplay{context}
				, img{Engine::Gfx::PixelFormat::RGB8, {512, 512}} {

				img.fill({255, 255, 0});
				tex.setAuto(img);
				setTexture(tex);
			}

			void requestRebuild() { nextRebuild = Engine::Clock::now() + rebuildDelay;}

		private:
			void rebuild() {
				nextRebuild = Engine::Clock::TimePoint::max();
				// Chunk offset isn't affected by zoom. Its a fixed offset so its always the same regardless of zoom.
				const auto chunkOffset = blockToChunk(offset);
				const auto res = img.size();

				// We can't use blockToChunk because we need ceil not floor.
				// Say we have a res = 512, zoom = 1.2, blocksPerChunk = 64: 512 * 1.2 = 614.4; 614.4 / 64 = 9.6
				// For blocks we would say that that block lives in the chunk floor(9.6) = 9, but this isn't blocks.
				// This is how many chunks will fit in the resolution. The res will hold 9.6 chunks so we need to generate ceil(9.6) = 10.
				const auto chunksPerImg = Engine::Math::divCeil(applyZoom<BlockVec>(res), blocksPerChunk).q;

				if (mode == Layer::Blocks) {
					terrain = {};
					std::atomic_signal_fence(std::memory_order_acq_rel); const auto startTime = Engine::Clock::now(); std::atomic_signal_fence(std::memory_order_acq_rel);
					generator.generate1(terrain, Terrain::Request{chunkOffset, chunkOffset + chunksPerImg, 0});
					std::atomic_signal_fence(std::memory_order_acq_rel); const auto endTime = Engine::Clock::now(); std::atomic_signal_fence(std::memory_order_acq_rel);
					ENGINE_LOG2("Generation Time: {}", Engine::Clock::Seconds{endTime - startTime});
				}

				// TODO: Move this color specification to Blocks.xpp, could be useful
				//       elsewhere. Alternatively, calculate this value based on the avg img
				//       color when loading them/packing them into atlas.
				static const auto blockToColor = []{
					std::array<glm::u8vec3, BlockId::_count> colors = {};
					colors[BlockId::Entity] = {  0, 120, 189};
					colors[BlockId::Debug]  = {255,   0,   0};
					colors[BlockId::Debug2] = {200,  26, 226};
					colors[BlockId::Debug3] = {226,  26, 162};
					colors[BlockId::Debug4] = {226,  26, 111};
					colors[BlockId::Dirt]	  = {158,  98,  33};
					colors[BlockId::Grass]  = { 67, 226,  71};
					colors[BlockId::Iron]	  = {144, 144, 144};
					colors[BlockId::Gold]	  = {255, 235,  65};
					return colors;
				}();

				static const auto biomeToColor = []{
					std::array<glm::u8vec3, decltype(generator)::getBiomeCount()> colors{};
					float32 hue = 0;
					for (auto& color : colors) {
						hue = Engine::Math::nextRandomHue(hue);
						color = Engine::Math::cvtFloatRGBToByteRGB(Engine::Math::cvtHSLtoRGB({hue, 0.85f, 0.5f}));
					}
					return colors;
				}();

				auto* data = reinterpret_cast<glm::u8vec3*>(img.data());

				const RealmId realmId = 0; // TODO: realm support
				for (BlockUnit y = 0; y < res.y; ++y) {
					const auto yspan = y * res.x;
					for (BlockUnit x = 0; x < res.x; ++x) {
						const auto blockCoord = offset + applyZoom(BlockVec{x, y});
						const auto idx = x + yspan;

						if (mode == Layer::BiomeRawWeights) {
							data[idx] = biomeToColor[generator.calcBiomeRaw(blockCoord).id];
						} else if (mode == Layer::BiomeBlendWeights) {
							auto weights = generator.calcBiomeBlend(blockCoord);
							normalizeBiomeWeights(weights);
							const auto biome = maxBiomeWeight(weights);
							data[idx] = glm::u8vec3(biome.weight * glm::vec3(biomeToColor[biome.id]));
						} else if (mode == Layer::BiomeFinalWeights) {
							auto weights = generator.calcBiome(blockCoord);
							const auto biome = maxBiomeWeight(weights);
							data[idx] = biomeToColor[biome.id];
						} else if (mode == Layer::BiomeFinalWeightsFull) {
							auto weights = generator.calcBiome(blockCoord);
							const auto biome = maxBiomeWeight(weights);
							data[idx] = glm::u8vec3(biome.weight * glm::vec3(biomeToColor[biome.id]));
						} else if (mode == Layer::Blocks) {
							const auto chunkCoord = blockToChunk(blockCoord);
							const auto regionCoord = chunkToRegion(chunkCoord);
							auto& region = terrain.getRegion({realmId, regionCoord});

							// Sanity checks
							ENGINE_DEBUG_ASSERT(blockCoord.x >= offset.x);
							ENGINE_DEBUG_ASSERT(blockCoord.y >= offset.y);

							// TODO: cant we just do chunkCoord - regionCoord.toChunk() which should be a lot cheaper?
							const auto chunkIndex = chunkToRegionIndex(chunkCoord, regionCoord);
							auto& chunk = region.chunks[chunkIndex.x][chunkIndex.y];
							ENGINE_DEBUG_ASSERT(region.stages[chunkIndex.x][chunkIndex.y] == generator.totalStages, "Chunk is at incorrect stage.");

							const auto blockIndex = blockToChunkIndex(blockCoord, chunkCoord);
							ENGINE_DEBUG_ASSERT(blockIndex.x >= 0 && blockIndex.x < chunkSize.x, "Invalid chunk index.");
							ENGINE_DEBUG_ASSERT(blockIndex.y >= 0 && blockIndex.y < chunkSize.y, "Invalid chunk index.");

							const auto blockId = chunk.data[blockIndex.x][blockIndex.y];
							ENGINE_DEBUG_ASSERT(blockId >= 0 && blockId < BlockId::_count, "Invalid BlockId.");

							data[idx] = blockToColor[blockId];
						}
					}
				}

				// TODO: why does fixed size not work here. Fixed size should always work.
				setFixedSize(img.size()); // TODO: rm - just for testing

				tex.setImage(img);
			};

			void render() override {
				if (Engine::Clock::now() > nextRebuild) {
					ENGINE_INFO2("Zoom: {}", zoom);
					rebuild();
				}

				ImageDisplay::render();
			}

			// TODO: custom render for timeout, maybe just use an update func?

			bool onAction(EUI::ActionEvent action) override {
				switch (action) {
					case EUI::Action::Scroll: {
						ENGINE_LOG2("action.value.f32: {}", action.value.f32);
						zoom -= action.value.f32 * 0.2;
						nextRebuild = Engine::Clock::now() + rebuildDelay;
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
		sec->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, theme.sizes.pad1});
		sec->setAutoSizeHeight(true);

		constexpr auto textGetter = []<class V>(V& var){ return [&var, last=V{}](EUI::TextBox& box) mutable {
			if (var == last) { return; }
			last = var;
			box.setText(std::to_string(last));
		};};

		const auto textSetter = [area](auto& var){ return [area, &var](EUI::TextBox& box) {
			std::from_chars(std::to_address(box.getText().begin()), std::to_address(box.getText().end()), var);
			area->requestRebuild();
		};};

		const auto xMove = ctx->createPanel<EUI::TextBox>(sec);
		xMove->autoSize();
		xMove->bind(textGetter(area->offset.x), textSetter(area->offset.x));

		const auto yMove = ctx->createPanel<EUI::TextBox>(sec);
		yMove->autoSize();
		yMove->bind(textGetter(area->offset.y), textSetter(area->offset.y));

		const auto layer = ctx->createPanel<EUI::Slider>(sec);
		layer->setLimits(0, +Layer::_last);
		layer->bind(
			[area](EUI::Slider& self){
				self.setValue(+area->mode);
			},
			[area](EUI::Slider& self){
				area->mode = static_cast<Layer>(std::round(self.getValue()));
				area->requestRebuild();
			}
		);

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
