// GLM
#include <glm/glm.hpp>

// Engine
#include <Engine/Noise/Noise.hpp>

// Game
#include <Game/MapGenerator2.hpp>
#include <Game/MapChunk.hpp>


namespace {
	using namespace Engine::Types;

	template<int N>
	constexpr float32 gradient(const float32 x, const glm::vec2 (&points)[N]) noexcept {
		static_assert(N > 0, "Can not interpolate between zero points");

		auto min = std::cbegin(points);
		if (x > min->x) { return min->y; }

		const auto last = std::cend(points);
		if (const auto i = last - 1; x < i->x) { return i->y; }

		auto max = min;
		while (min != last) {
			if (x > min->x) {
				const float32 p = (x - min->x) / (max->x - min->x);
				return min->y + p * (max->y - min->y);
			}
			max = min;
			++min;
		}

		return max->y;
	}

	ENGINE_INLINE constexpr Game::MapGenerator2::Float operator""_f(const long double v) noexcept {
		return static_cast<Game::MapGenerator2::Float>(v);
	}
}

#define DEF_BIOME_HEIGHT_OFFSET(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE auto MapGenerator2::biomeHeightOffset<MapGenerator2::Biome:: B>(const Float x) const noexcept -> Float

#define DEF_BIOME_HEIGHT_STRENGTH(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE auto MapGenerator2::biomeHeightStrength<MapGenerator2::Biome:: B>(const Float x, const BiomeBounds bounds) const noexcept -> Float

#define DEF_BIOME_BASIS(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE auto MapGenerator2::biomeBasis<MapGenerator2::Biome:: B>(const FVec2 pos, const Int h) const noexcept -> Float

#define DEF_BIOME_BASIS_STRENGTH(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE auto MapGenerator2::biomeBasisStrength<MapGenerator2::Biome:: B>(const FVec2 pos, const FVec2 posBiome, const BiomeBounds bounds) const noexcept -> Float

#define DEF_BIOME_BLOCK(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE BlockId MapGenerator2::biomeBlock<MapGenerator2::Biome:: B>(const FVec2 pos, const IVec2 ipos, const Int h, const Float h0, const BiomeBounds bounds) const noexcept

#define DEF_BIOME_BLOCK_STRENGTH(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE auto MapGenerator2::biomeBlockStrength<MapGenerator2::Biome:: B>(const FVec2 pos, const Float basisStrength) const noexcept -> bool

////////////////////////////////////////////////////////////////////////////////
// Biome 0
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(Default) {
		return 15 * simplex.value(x * 0.05_f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(Default) {
		return 1.0_f;
	}

	DEF_BIOME_BASIS(Default) {
		if (pos.y > h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr Float groundScale = 1.0_f / 100.0_f;
		const Float groundGrad = std::max(0.0_f, 1.0_f - (h - pos.y) * groundScale);
		return simplex.value(pos * 0.06_f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(Default) {
		return 1;
	}

	DEF_BIOME_BLOCK(Default) {
		// Add grass to "top" layer
		if (pos.y > -heightVar) {
			if ((h == ipos.y)
				// Really this should resample biomes and h0 to get correct bounds.
				// But this is much cheaper and works basically all the time anyways.
				//|| (height<Biome::Jungle>(pos.x - 1, bounds, h0) < ipos.y)
				//|| (height<Biome::Jungle>(pos.x + 1, bounds, h0) < ipos.y)
				) {
				return BlockId::Grass;
			}
		}

		if (const auto r = resource(pos)) {
			return r;
		}

		return BlockId::Dirt;
	}

	DEF_BIOME_BLOCK_STRENGTH(Default) {
		return 1.0_f;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Biome 1
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(Forest) {
		return 85 * simplex.value(x * 0.01_f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(Forest) {
		return genericBiomeHeightStrength(x, bounds);
	}

	DEF_BIOME_BASIS(Forest) {
		if (pos.y > h) { return -1.0_f; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr Float groundScale = 1.0_f / 100.0_f;
		const Float groundGrad = std::max(0.0_f, 1.0_f - (h - pos.y) * groundScale);
		return simplex.value(pos * 0.01_f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(Forest) {
		return genericBiomeBasisStrength(posBiome, bounds);
	}

	DEF_BIOME_BLOCK(Forest) {
		return BlockId::Debug + (BlockId)bounds.depth;
	}

	DEF_BIOME_BLOCK_STRENGTH(Forest) {
		return genericBiomeBlockStrength(pos, basisStrength);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Biome 2
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(Jungle) {
		return 85 * simplex.value(x * 0.01_f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(Jungle) {
		return genericBiomeHeightStrength(x, bounds);
	}

	DEF_BIOME_BASIS(Jungle) {
		if (pos.y > h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr Float groundScale = 1.0_f / 100.0_f;
		const Float groundGrad = std::max(0.0_f, 1.0_f - (h - pos.y) * groundScale);
		return simplex.value(pos * 0.005_f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(Jungle) {
		return genericBiomeBasisStrength(posBiome, bounds);
	}

	DEF_BIOME_BLOCK(Jungle) {
		return BlockId::Debug + (BlockId)bounds.depth;
	}

	DEF_BIOME_BLOCK_STRENGTH(Jungle) {
		return genericBiomeBlockStrength(pos, basisStrength);
	}
}
#undef DEF_BIOME_HEIGHT_OFFSET
#undef DEF_BIOME_HEIGHT_STRENGTH
#undef DEF_BIOME_BASIS
#undef DEF_BIOME_BASIS_STRENGTH
#undef DEF_BIOME_BLOCK
#undef DEF_BIOME_BLOCK_STRENGTH

#define DEF_LANDMARK_SAMPLE(L)\
	template<>\
	auto MapGenerator2::landmarkSample<MapGenerator2::Landmark:: L>(const FVec2 pos, const IVec2 ipos, const Int h, BlockGenData& bgd) const noexcept -> LandmarkSample

////////////////////////////////////////////////////////////////////////////////
// Landmark 0
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_LANDMARK_SAMPLE(TreeDefault) {
		if (ipos.y <= h) { return { .exists = false }; }

		constexpr Float treeSpacing = 11; // Average spacing between tree centers
		constexpr Int treeThresh = 200; // Chance for a tree to exist at this spacing out of 255
		const auto left = Engine::Noise::floorTo<Int>(ipos.x * (1.0_f / treeSpacing)); // Use the left grid cell to determine trees

		if (perm.value(left) < treeThresh) {
			const Int treeH = h + 25 + static_cast<Int>(perm.value(left - 321) * (50.0_f/255.0_f));
			if (ipos.y < treeH) {
				constexpr Int trunkD = 3; // Trunk width // TODO: doesnt work correctly for even width.
				constexpr Int trunkR = (trunkD / 2) + (trunkD / 2.0_f != trunkD / 2);
				constexpr Float pad = (0 + trunkR) * (1.0_f/treeSpacing);

				// If the padding is near 0.5 you might as well use fixed spacing
				static_assert(pad < 0.45_f, "Tree width is to large relative to tree spacing");

				const Float a = left + pad;
				const Float b = left + 1.0_f - pad;
				const Float off = perm.value(left + 321) * (1.0_f/255.0_f);
				const Int wpos = Engine::Noise::floorTo<Int>((a + off * (b - a)) * treeSpacing);
				if (ipos.x < (wpos + trunkR) && ipos.x > (wpos - trunkR)) {
					if (ipos.x == wpos && ipos.y == h + 1) {
						bgd.exists = true;
						bgd.desc.data.type = BlockEntityType::Tree;
						bgd.desc.data.asTree.type = perm(ipos.x) % 3;
						bgd.desc.data.asTree.size = {
							trunkD,
							treeH - h,
						};
					}

					return { .exists = true, .basis = 1.0_f, .block = BlockId::Debug2 };
				}
			}
		}
				
		return { .exists = false };
	}
}

////////////////////////////////////////////////////////////////////////////////
// Landmark 1
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_LANDMARK_SAMPLE(TreeForest) {
		return { .exists = false };
	}
}

////////////////////////////////////////////////////////////////////////////////
// Landmark 2
////////////////////////////////////////////////////////////////////////////////
namespace Game {
}

////////////////////////////////////////////////////////////////////////////////
// Landmark 3
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_LANDMARK_SAMPLE(BossPortal) {
		constexpr Int w = 50;
		constexpr Float d = 1.0_f / w;
		const auto scaled = pos * d;
		const auto cell = glm::floor(scaled);
		// TODO: we will want this to be a much smaller than 1%. Is there a function we could use instead of a perm table?
		// TODO: cont. Could we just use a LCG and seed from cell.x and cell.y? look into other hash functions
		if (perm(static_cast<int>(cell.x), static_cast<int>(cell.y), static_cast<int>(Landmark::BossPortal)) > 5) { return { .exists = false }; }
		const auto offC = 2.0_f * (scaled - cell) - 1.0_f; // Offset from center [-1, 1]
		const auto grad = 2 * glm::min(glm::length2(offC)*glm::length2(offC), 1.0_f); // Circle grad [0, 2]

		// floor level
		if (offC.y > -0.3_f) {
			if (offC.y < 0.5_f && offC.x < 0.2_f && offC.x > -0.2_f) {
				return {.exists = true, .basis = 1.0_f, .block = BlockId::Debug4 };
			}
			return {.exists = true, .basis = grad - 2.0_f, .block = BlockId::None };
		}
		// TODO: we would really like some blending around left and right sides for larger values
		return {.exists = true, .basis = 2.0_f - grad, .block = BlockId::None };
	}
}

#undef DEF_LANDMARK_SAMPLE

namespace Game {
	void MapGenerator2::init(const IVec2 pos, MapChunk& chunk, std::vector<BlockEntityDesc>& entData) const noexcept {
		BlockGenData bgd = { .exists = false };

		for (int x = 0; x < MapChunk::size.x; ++x) {
			for (int y = 0; y < MapChunk::size.y; ++y) {
				const auto blockPos = pos + IVec2{x,y};
				const auto v = value(blockPos.x, blockPos.y, bgd);

				if (bgd.exists) {
					bgd.desc.pos = blockPos;
					entData.push_back(bgd.desc);
					chunk.data[x][y] = BlockId::Entity;
					bgd.exists = false;
				} else {
					chunk.data[x][y] = v;
				}

				if (chunk.data[x][y] > BlockId::Air && (x == 0 || y == 0) && chunk.data[x][y] != BlockId::Debug2) {
					chunk.data[x][y] = BlockId::Debug;
				}
			}
		}
	}

	BlockId MapGenerator2::value(const Int x, const Int y, BlockGenData& bgd) const noexcept {
		const FVec2 ipos = IVec2{x, y};
		const FVec2 pos = FVec2{static_cast<Float>(x), static_cast<Float>(y)};
		const auto h0 = height0(pos.x);
		const auto posBiome = pos - FVec2{0, h0} - biomeOffset;
		const auto bounds = biomeAt(posBiome);

		const Biome b = bounds.depth < 0 ? Biome::Default : static_cast<Biome>(perm(bounds.cell.x, bounds.cell.y) % (static_cast<int>(Biome::Jungle) + 1));
		
		#define CASE(B) case B: { return calc<B>(ipos, pos, h0, posBiome, bounds, bgd); };
		switch (b) {
			CASE(Biome::Default)
			CASE(Biome::Forest)
			CASE(Biome::Jungle)
			default: {
				ENGINE_WARN("Unknown biome ", static_cast<int>(b));
				return BlockId::Debug;
			}
		}
		#undef CASE
	}

	template<MapGenerator2::Biome B>
	BlockId MapGenerator2::calc(const IVec2 ipos, const FVec2 pos, const Float h0, const FVec2 posBiome, const BiomeBounds bounds, BlockGenData& bgd) const noexcept {
		const auto h = height<B>(pos.x, bounds, h0);

		const auto bstr = basisStrength<B>(pos, posBiome, bounds);
		const auto b = basis<B>(pos, h, bstr);

		// TODO: move before basis stuff if we have l2.block != None
		const auto l2 = landmark<B>(pos, ipos, h, bgd);
		if (l2.exists) {
			const auto b2 = b + l2.basis;
			if (b2 < 0.0_f) {
				//return BlockId::Debug2;
				return BlockId::Air;
			} else {
				return l2.block ? l2.block : block<B>(pos, ipos, h, h0, bounds, bstr);
			}
		}

		if (b <= 0.0_f) {
			return BlockId::Air;
		};

		return block<B>(pos, ipos, h, h0, bounds, bstr);
	}

	BlockId MapGenerator2::resource(const FVec2 pos) const noexcept {
		struct ResourceSpec {
			consteval ResourceSpec(BlockId b, Float s, Float d)
				: block{b}, scale{1.0_f/s}, density{d * 2.0_f - 1.0_f} {
			}

			BlockId block = BlockId::None;
			Float scale = 1.0_f;
			Float density = 0.0_f;
		};

		// TODO: will probably want to be able to set biome/depth requirements
		constexpr ResourceSpec ores[] = {
			{BlockId::Gold, 7.0_f, 0.11_f},
			{BlockId::Iron, 5.0_f, 0.2_f},
		};

		for (const auto& ore : ores) {
			const auto p = pos * ore.scale;
			if (simplex.value(p.x, p.y) < ore.density) {
				return ore.block;
			}
		}

		return BlockId::None;
	}

	/*
	Int MapGenerator2::height(const FVec2 pos) const noexcept {
		auto octave = [&](const Float xs, const Float ys, const Float off, const Float one = 0.0_f) ENGINE_INLINE {
			// TODO: use 1d simplex when you get it workign correctly
			return ys * 0.5_f * (one + simplex.value(xs * (pos.x + off), 0));
		};

		Float h = 0.0_f;

		// TODO: this may be to much variation? Might make it hard to do anything in the sky. Then again we could just offset by `basis + #`
		h += octave(0.0001_f, 500.0_f, 1 * 999.0_f, 1.0_f);
		h += octave(0.005_f, 50.0_f, 2 * 999.0_f);
		h += octave(0.05_f, 5.0_f, 3 * 999.0_f);
		h += octave(0.75_f, 2.0_f, 4 * 999.0_f);
		return static_cast<Int>(h);
	}*/

	auto MapGenerator2::biomeAt(const FVec2 pos) const noexcept -> BiomeBounds {
		for (int l = 0; l < std::size(biomeScales); ++l) {
			const IVec2 cell = glm::floor(pos * biomeScalesInv[l]);
			if (perm(cell.x, cell.y) < 10) {
				//return BlockId::Debug + static_cast<BlockId>(l);
				return {l, cell};
			}
		}

		return {-1};
	}

	auto MapGenerator2::height0(const Float x) const noexcept -> Float {
		return heightVar * simplex.value(0.000005_f * x, 0); // TODO: 1d simplex
	}
	
	template<MapGenerator2::Biome B>
	auto MapGenerator2::height(const Float x, const BiomeBounds bounds, const Float h0) const noexcept -> Int {
		const auto hOff1 = biomeHeightOffset<Biome::Default>(x);
		auto h = hOff1;

		if constexpr (B != Biome::Default) {
			if (bounds.depth >= 0) {
				const auto hOff2 = biomeHeightOffset<B>(x);
				const auto b2s = biomeHeightStrength<B>(x, bounds);
				h = hOff1 + b2s * (hOff2 - hOff1);
			}
		}

		return static_cast<Int>(h + h0);
	}
	
	template<MapGenerator2::Biome B>
	auto MapGenerator2::landmark(const FVec2 pos, const IVec2 ipos, const Int h, BlockGenData& bgd) const noexcept -> LandmarkSample {
		LandmarkSample res = {};

		const auto sample = [&]<Landmark L>() ENGINE_INLINE {
			res = landmarkSample<L>(pos, ipos, h, bgd);
		};

		constexpr auto iter = [&]<class T, T... Is>(std::integer_sequence<T, Is...>) ENGINE_INLINE {
			((sample.template operator()<landmarksByBiome<B>[Is]>(), res.exists) || ...);
		};

		iter(std::make_index_sequence<landmarksByBiome<B>.size()>{});
		return res;
	}

	template<MapGenerator2::Biome B>
	auto MapGenerator2::basisStrength(const FVec2 pos, const FVec2 posBiome, const BiomeBounds bounds) const noexcept -> Float {
		if constexpr (B != Biome::Default) {
			if (bounds.depth >= 0) {
				return biomeBasisStrength<B>(pos, posBiome, bounds);
			}
		}
		return 0.0_f;
	}
	
	template<MapGenerator2::Biome B>
	auto MapGenerator2::basis(const FVec2 pos, const Int h, const Float bstr) const noexcept -> Float {
		if constexpr (B != Biome::Default) {
			if (bstr > 0.0_f) {
				const auto b2 = biomeBasis<B>(pos, h);
				if (bstr >= 1.0_f) { return b2; }

				const auto b1 = biomeBasis<Biome::Default>(pos, h);
				return b1 + bstr * (b2 - b1);
			}
		}

		return biomeBasis<Biome::Default>(pos, h);
	}
	
	template<MapGenerator2::Biome B>
	BlockId MapGenerator2::block(const FVec2 pos, const FVec2 ipos, const Int h, const Float h0, const BiomeBounds bounds, const Float bstr) const noexcept {
		if constexpr (B != Biome::Default) {
			if (bounds.depth >= 0 && biomeBlockStrength<B>(pos, bstr)) {
				return biomeBlock<B>(pos, ipos, h, h0, bounds);
			}
		}

		return biomeBlock<Biome::Default>(pos, ipos, h, h0, bounds);
	}

	/*
	Int MapGenerator2::biome(const FVec2 pos) const noexcept {
		// Perturb
		const auto p = simplex.value(pos.x * 0.0006_f, pos.y * 0.0006_f);
		const auto p2 = pos * 0.0003_f + p * 0.3_f;

		// Edge noise
		const auto m = simplex.value(pos.x * 0.3_f, pos.y * 0.3_f);

		// Main shape
		const auto wv = worley.valueF2F1(p2.x, p2.y);

		// Combine
		const auto v = wv.value - m * 0.08_f;

		return v < 0.2_f ? 0 : perm(wv.cell.x, wv.cell.y);
	}*/

	auto MapGenerator2::genericBiomeHeightStrength(const Float x, const BiomeBounds bounds) const noexcept -> Float {
		// TODO: we use this radius/offset type logic in a number of places. Probably extract and pass as args.
		const auto s = biomeScales[bounds.depth];
		const auto center = (bounds.cell.x + 0.5_f) * s;
		const auto off = 0.5_f * s - std::abs(center - x);
		constexpr auto tDist = 1.0_f / 350.0_f; // 1 / transition distance in blocks
		return std::min(1.0_f, off * tDist);
	}

	auto MapGenerator2::genericBiomeBasisStrength(const FVec2 posBiome, const BiomeBounds bounds) const noexcept -> Float {
		// Could do circle instead if that looks better - `1 - length(off)` instead of compMin 
		const auto s = biomeScales[bounds.depth];
		const auto center = (FVec2{bounds.cell} + 0.5_f) * s;
		const auto off = 0.5_f * s - glm::abs(center - posBiome);
		// TODO: adjust transition distance based on bounds.depth. Currently small biomes its to large and large biomes its to small. probably want something like 200, and 600
		// TODO: we use this same tDist in height str. probably want it in a central location.
		constexpr auto tDist = 1.0_f / 350.0_f; // 1 / transition distance in blocks
		return glm::compMin(off * tDist);
	}
	
	auto MapGenerator2::genericBiomeBlockStrength(const FVec2 pos, const Float basisStrength) const noexcept -> bool {
		auto adj = basisStrength;
		adj += 0.3_f * simplex.value(0.03_f * pos);
		adj += 0.3_f * simplex.value(0.09_f * pos);
		return 0.5_f < adj;
	}
}
