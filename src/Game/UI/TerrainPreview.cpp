// Game
#include <Game/UI/TerrainPreview.hpp>
#include <Game/TerrainGenerator.hpp>
#include <Game/Terrain/biomes/BiomeOne.hpp>

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
	using namespace Game::Terrain;

	enum class Layer {
		BiomeBaseGrid,
		BiomeRawWeights,
		BiomeBlendWeights,
		BiomeFinalWeights,
		BiomeFinalWeightsFull,
		TerrainHeight0,
		//TerrainHeight1,
		Blocks,
		_count,
		_last = _count - 1,
	};
	ENGINE_BUILD_DECAY_ENUM(Layer);

	template<uint64 Seed>
	struct BiomeDebugBase {
		Engine::Noise::OpenSimplexNoise simplex1{Engine::Noise::lcg(Seed)};
		Engine::Noise::OpenSimplexNoise simplex2{Engine::Noise::lcg(Engine::Noise::lcg(Seed))};
		Engine::Noise::OpenSimplexNoise simplex3{Engine::Noise::lcg(Engine::Noise::lcg(Engine::Noise::lcg(Seed)))};

		// TODO: create and example/concept/check with only the _needed_/optional functions.
		//       getBasisHeightBlend is not part of the TerrainGenerator requirements.
		Float getBasisHeightBlend(const BlockUnit h0, const BlockUnit y) {
			return (200 - std::min<BlockUnit>(h0 - y, 200)) * (1.0_f / 200.0_f);
		}

		Float getBasisStrength(TERRAIN_GET_BASIS_STRENGTH_ARGS) {
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
			if (blockCoord.y > h0) { return 0.0f; }
			return getBasisHeightBlend(h0, blockCoord.y) + std::abs(simplex1.value(glm::vec2{blockCoord} * 0.03_f)) - 0.15_f;
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
			if (blockCoord.y > h0) { return 0.0f; }
			return getBasisHeightBlend(h0, blockCoord.y) + std::abs(simplex1.value(glm::vec2{blockCoord} * 0.06_f)) - 0.75_f;
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
			if (blockCoord.y > h0) { return 0.0f; }
			return getBasisHeightBlend(h0, blockCoord.y) + simplex1.value(glm::vec2{blockCoord} * 0.12_f);
		}
	};

	class TerrainDragArea : public EUI::ImageDisplay {
		public:
			BlockVec offset = {80367, 525};
			//BlockVec offset = {77967.0, 0.0};
			//BlockVec offset = {-127, -69};

			float64 zoom = 5.0f; // Larger # = farther out = see more = larger FoV
			Layer mode = Layer::BiomeRawWeights;

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
			//Terrain::Generator<BiomeDebugOne, BiomeDebugTwo, BiomeDebugThree> generator{1234};
			Generator<BiomeOne, BiomeDebugOne, BiomeDebugTwo, BiomeDebugThree> generator{1234};
			Game::Terrain::Terrain terrain;

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
				std::atomic_signal_fence(std::memory_order_acq_rel); const auto startTime = Engine::Clock::now(); std::atomic_signal_fence(std::memory_order_acq_rel);

				nextRebuild = Engine::Clock::TimePoint::max();
				// Chunk offset isn't affected by zoom. Its a fixed offset so its always the same regardless of zoom.
				const auto chunkOffset = blockToChunk(offset);
				const auto res = img.size();

				// We can't use blockToChunk because we need ceil not floor.
				// Say we have a res = 512, zoom = 1.2, blocksPerChunk = 64: 512 * 1.2 = 614.4; 614.4 / 64 = 9.6
				// For blocks we would say that that block lives in the chunk floor(9.6) = 9, but this isn't blocks.
				// This is how many chunks will fit in the resolution. The res will hold 9.6 chunks so we need to generate ceil(9.6) = 10.
				const auto chunksPerImg = Engine::Math::divCeil(applyZoom<BlockVec>(res), blocksPerChunk).q;
				const auto indexToBlock = [&](BlockVec index) ENGINE_INLINE { return offset + applyZoom(index); };

				if (mode == Layer::Blocks) {
					terrain = {};
					generator.generate1(terrain, Request{chunkOffset, chunkOffset + chunksPerImg, 0});
				} else {
					// The height cache is still needs to be populated for biome sampling.
					generator.populateHeight0Cache(indexToBlock({0, 0}).x - biomeBlendDist, indexToBlock(res).x + biomeBlendDist);
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
					colors[BlockId::Dirt]	= {158,  98,  33};
					colors[BlockId::Grass]  = { 67, 226,  71};
					colors[BlockId::Iron]	= {144, 144, 144};
					colors[BlockId::Gold]	= {255, 235,  65};
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

				static const auto sizeToBrightness = [](BlockUnit size){
					return 0.5_f + size / (2.0_f * biomeScales[0].size);
				};

				auto* data = reinterpret_cast<glm::u8vec3*>(img.data());
				const auto& heightCache = generator.getHeightCache();

				const RealmId realmId = 0; // TODO: realm support
				for (BlockUnit y = 0; y < res.y; ++y) {
					const auto yspan = y * res.x;
					for (BlockUnit x = 0; x < res.x; ++x) {
						const auto blockCoord = indexToBlock({x, y});
						const auto idx = x + yspan;

						if (mode == Layer::BiomeBaseGrid) {
							// This won't line up 100% because we don't include the height offset (see
							// BiomeRawWeights), but that's the point. Showing the undistorted biome grid.
							const auto blockCoordAdj = blockCoord - biomeScaleOffset;
							const auto info = generator.calcBiomeRaw(blockCoordAdj);
							data[idx] = sizeToBrightness(info.meta->size) * glm::vec3(biomeToColor[info.id]);
						} else if (mode == Layer::BiomeRawWeights) {
							// Need to include the biome offset or else things won't line up when switching layers.
							const auto blockCoordAdj = blockCoord - (biomeScaleOffset + heightCache.get(blockCoord.x));
							const auto info = generator.calcBiomeRaw(blockCoordAdj);
							data[idx] = sizeToBrightness(info.meta->size) * glm::vec3(biomeToColor[info.id]);
						} else if (mode == Layer::BiomeBlendWeights) {
							auto weights = generator.calcBiomeBlend(blockCoord);
							normalizeBiomeWeights(weights);
							const auto biome = maxBiomeWeight(weights);
							data[idx] = biome.weight * glm::vec3(biomeToColor[biome.id]);
						} else if (mode == Layer::BiomeFinalWeights) {
							auto weights = generator.calcBiome(blockCoord);
							const auto biome = maxBiomeWeight(weights);
							data[idx] = biomeToColor[biome.id];
						} else if (mode == Layer::BiomeFinalWeightsFull) {
							auto weights = generator.calcBiome(blockCoord);
							const auto biome = maxBiomeWeight(weights);
							data[idx] = biome.weight * glm::vec3(biomeToColor[biome.id]);
						} else if (mode == Layer::TerrainHeight0) {
							auto weights = generator.calcBiome(blockCoord);
							const auto biome = maxBiomeWeight(weights);

							// We need to clamp blockCoord at maxBlock because of the applyZoom
							// does ceil which can give slightly off results from some values of
							// zoom < 1 due to float precision.
							const auto h0 = heightCache.get(std::min(heightCache.getMaxBlock() - 1, blockCoord.x));
							data[idx] = blockCoord.y <= h0 ? glm::u8vec3(biome.weight * glm::vec3(biomeToColor[biome.id])) : glm::u8vec3{};
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

						BlockVec poi = {80670, 889};
						const auto tol = 2 * zoom; // The zoom multiplier gives use a x*x pixel dot at the point.
						if ((blockCoord.x >= poi.x) && (blockCoord.x <= poi.x + tol) && (blockCoord.y >= poi.y) && (blockCoord.y <= poi.y + tol)) {
							data[idx] = {200, 0, 0};
						}
					}
				}

				std::atomic_signal_fence(std::memory_order_acq_rel); const auto endTime = Engine::Clock::now(); std::atomic_signal_fence(std::memory_order_acq_rel);
				ENGINE_LOG2("Generation Time: {}", Engine::Clock::Seconds{endTime - startTime});

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

		//
		//
		//
		//
		//
		//
		// TODO: why does this refresh instantly? Where is the delay?
		//
		//
		//
		//
		//
		//
		//
		//
		//
		//
		const auto zoom = ctx->createPanel<EUI::TextBox>(sec);
		zoom->autoSize();
		zoom->bind(textGetter(area->zoom), textSetter(area->zoom));

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
