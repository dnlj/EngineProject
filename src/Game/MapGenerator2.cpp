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
}

#define DEF_BIOME_HEIGHT_OFFSET(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeHeightOffset<MapGenerator2::Biome:: B>(const float32 x) const noexcept

#define DEF_BIOME_HEIGHT_STRENGTH(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeHeightStrength<MapGenerator2::Biome:: B>(const float32 x, const BiomeBounds bounds) const noexcept

#define DEF_BIOME_BASIS(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeBasis<MapGenerator2::Biome:: B>(const glm::vec2 pos, const int32 h) const noexcept

#define DEF_BIOME_BASIS_STRENGTH(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeBasisStrength<MapGenerator2::Biome:: B>(const glm::vec2 pos, const glm::vec2 posBiome, const BiomeBounds bounds) const noexcept

#define DEF_BIOME_BLOCK(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE BlockId MapGenerator2::biomeBlock<MapGenerator2::Biome:: B>(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h, const float32 h0, const BiomeBounds bounds) const noexcept

#define DEF_BIOME_BLOCK_STRENGTH(B)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeBlockStrength<MapGenerator2::Biome:: B>(const float32 basisStrength) const noexcept

////////////////////////////////////////////////////////////////////////////////
// Biome 0
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(Default) {
		return 15 * simplex.scaled(x * 0.05f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(Default) {
		return 1.0f;
	}

	DEF_BIOME_BASIS(Default) {
		if (pos.y > h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr float32 groundScale = 1.0f / 100.0f;
		const float32 groundGrad = std::max(0.0f, 1.0f - (h - pos.y) * groundScale);
		return simplex.scaled(pos * 0.06f) + groundGrad;
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
		return 1.0f;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Biome 1
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(Forest) {
		return 85 * simplex.scaled(x * 0.01f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(Forest) {
		// TODO: we use this radius/offset type logic in a number of places. Probably extract and pass as args.
		const auto s = biomeScales[bounds.depth];
		const auto center = (bounds.cell.x + 0.5f) * s;
		const auto off = 0.5f * s - std::abs(center - x + biomeOffset.x);
		constexpr auto tDist = 1.0f / 350.0f; // 1 / transition distance in blocks
		return std::min(1.0f, off * tDist);
	}

	DEF_BIOME_BASIS(Forest) {
		if (pos.y > h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr float32 groundScale = 1.0f / 100.0f;
		const float32 groundGrad = std::max(0.0f, 1.0f - (h - pos.y) * groundScale);
		return simplex.scaled(pos * 0.01f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(Forest) {
		const auto s = biomeScales[bounds.depth];
		const auto center = (glm::vec2{bounds.cell} + 0.5f) * s;
		const auto off = 0.5f * s - glm::abs(center - posBiome + biomeOffset);
		constexpr auto tDist = 1.0f / 350.0f; // 1 / transition distance in blocks
		return glm::compMin(glm::min(off * tDist, 1.0f));
	}

	DEF_BIOME_BLOCK(Forest) {
		return BlockId::Debug + (BlockId)bounds.depth;
	}

	DEF_BIOME_BLOCK_STRENGTH(Forest) {
		return 0.5f < basisStrength;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Biome 2
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(Jungle) {
		return 85 * simplex.scaled(x * 0.01f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(Jungle) {
		// TODO: we use this radius/offset type logic in a number of places. Probably extract and pass as args.
		const auto s = biomeScales[bounds.depth];
		const auto center = (bounds.cell.x + 0.5f) * s;
		const auto off = 0.5f * s - std::abs(center - x + biomeOffset.x);
		constexpr auto tDist = 1.0f / 350.0f; // 1 / transition distance in blocks
		return std::min(1.0f, off * tDist);
	}

	DEF_BIOME_BASIS(Jungle) {
		if (pos.y > h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr float32 groundScale = 1.0f / 100.0f;
		const float32 groundGrad = std::max(0.0f, 1.0f - (h - pos.y) * groundScale);
		return simplex.scaled(pos * 0.005f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(Jungle) {
		const auto s = biomeScales[bounds.depth];
		const auto center = (glm::vec2{bounds.cell} + 0.5f) * s;
		const auto off = 0.5f * s - glm::abs(center - posBiome + biomeOffset);
		constexpr auto tDist = 1.0f / 350.0f; // 1 / transition distance in blocks
		return glm::compMin(glm::min(off * tDist, 1.0f));
	}

	DEF_BIOME_BLOCK(Jungle) {
		return BlockId::Debug + (BlockId)bounds.depth;
	}

	DEF_BIOME_BLOCK_STRENGTH(Jungle) {
		return 0.5f < basisStrength;
	}
}
#undef DEF_BIOME_HEIGHT_OFFSET
#undef DEF_BIOME_HEIGHT_STRENGTH
#undef DEF_BIOME_BASIS
#undef DEF_BIOME_BASIS_STRENGTH
#undef DEF_BIOME_BLOCK
#undef DEF_BIOME_BLOCK_STRENGTH

#define DEF_LANDMARK_BASIS(L)\
	template<>\
	auto MapGenerator2::landmarkBasis<MapGenerator2::Landmark:: L>(const glm::vec2 pos, const glm::ivec2 ipos) const noexcept -> LandmarkSample

#define DEF_LANDMARK_BLOCK(L)\
	template<>\
	BlockId MapGenerator2::landmarkBlock<MapGenerator2::Landmark:: L>(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h) const noexcept

////////////////////////////////////////////////////////////////////////////////
// Landmark 0
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_LANDMARK_BASIS(TreeDefault) {
		constexpr int32 w = 25;
		constexpr float32 d = 1.0f / w;
		const auto scaled = pos * d;
		const auto cell = glm::floor(scaled);
		if (perm(static_cast<int>(cell.x), static_cast<int>(cell.y), static_cast<int>(Landmark::TreeDefault)) > 5) { return { .exists = false }; }
		return { .basis = 1.0f };
	}

	DEF_LANDMARK_BLOCK(TreeDefault) {
		return BlockId::Debug3;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Landmark 1
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_LANDMARK_BASIS(TreeForest) {
		return { .exists = false };
	}

	DEF_LANDMARK_BLOCK(TreeForest) {
		return BlockId::None;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Landmark 2
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	//DEF_LANDMARK_BLOCK(TreeJungle) {
	//	return BlockId::None;
	//}
}

////////////////////////////////////////////////////////////////////////////////
// Landmark 3
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_LANDMARK_BASIS(BossPortal) {
		constexpr int32 w = 50;
		constexpr float32 d = 1.0f / w;
		const auto scaled = pos * d;
		const auto cell = glm::floor(scaled);
		// TODO: we will want this to be a much smaller than 1%. Is there a function we could use instead of a perm table?
		// TODO: cont. Could we just use a LCG and seed from cell.x and cell.y? look into other hash functions
		if (perm(static_cast<int>(cell.x), static_cast<int>(cell.y), static_cast<int>(Landmark::BossPortal)) > 5) { return { .exists = false }; }
		const auto offC = 2.0f * (scaled - cell) - 1.0f; // Offset from center [-1, 1]
		const auto grad = 2 * glm::min(glm::length2(offC)*glm::length2(offC), 1.0f); // Circle grad [0, 2]

		// floor level
		if (offC.y > -0.3f) {
			if (offC.y < 0.5f && offC.x < 0.2f && offC.x > -0.2f) {
				return {.exists = true, .basis = 1.0f, .block = BlockId::Debug4 };
			}
			return {.exists = true, .basis = grad - 2.0f, .block = BlockId::None };
		}
		// TODO: we would really like some blending around left and right sides for larger values
		return {.exists = true, .basis = 2.0f - grad, .block = BlockId::None };
	}

	DEF_LANDMARK_BLOCK(BossPortal) {
		return BlockId::Debug4;
	}
}

#undef DEF_LANDMARK_BASIS
#undef DEF_LANDMARK_BLOCK

namespace Game {
	void MapGenerator2::init(const glm::ivec2 pos, MapChunk& chunk) const noexcept {
		for (int x = 0; x < MapChunk::size.x; ++x) {
			for (int y = 0; y < MapChunk::size.y; ++y) {
				chunk.data[x][y] = value(x + pos.x, y + pos.y);

				if (chunk.data[x][y] > BlockId::Air && (x == 0 || y == 0) && chunk.data[x][y] != BlockId::Debug2) {
					chunk.data[x][y] = BlockId::Debug;
				}
			}
		}
	}

	BlockId MapGenerator2::value(const int32 x, const int32 y) const noexcept {
		const glm::vec2 ipos = glm::ivec2{x, y};
		const glm::vec2 pos = glm::vec2{static_cast<float32>(x), static_cast<float32>(y)};
		const auto h0 = height0(pos.x);
		const auto posBiome = pos - glm::vec2{0, h0};
		const auto bounds = biomeAt(posBiome);

		const Biome b = bounds.depth < 0 ? Biome::Default : static_cast<Biome>(perm(bounds.cell.x, bounds.cell.y) % (static_cast<int>(Biome::Jungle) + 1));
		
		#define CASE(B) case B: { return calc<B>(ipos, pos, h0, posBiome, bounds); };
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
	BlockId MapGenerator2::calc(const glm::ivec2 ipos, const glm::vec2 pos, const float32 h0, const glm::vec2 posBiome, const BiomeBounds bounds) const noexcept {
		const auto h = height<B>(pos.x, bounds, h0);
		
		if (const auto l = landmark(pos, ipos, h); l) {
			return l;
		}

		const auto bstr = basisStrength<B>(pos, posBiome, bounds);
		const auto b = basis<B>(pos, h, bstr);


		const auto l2 = landmark2<B>(pos, ipos, h);
		if (l2.exists) {
			const auto b2 = b + l2.basis;
			if (b2 < 0.0f) {
				//return BlockId::Debug2;
				return BlockId::Air;
			} else {
				return l2.block ? l2.block : block<B>(pos, ipos, h, h0, bounds, bstr);
			}
		}

		if (b <= 0.0f) {
			return BlockId::Air;
		};

		return block<B>(pos, ipos, h, h0, bounds, bstr);
	}

	BlockId MapGenerator2::resource(const glm::vec2 pos) const noexcept {
		struct ResourceSpec {
			consteval ResourceSpec(BlockId b, float32 s, float32 d)
				: block{b}, scale{1.0f/s}, density{d * 2.0f - 1.0f} {
			}

			BlockId block = BlockId::None;
			float32 scale = 1.0f;
			float32 density = 0.0f;
		};

		// TODO: will probably want to be able to set biome/depth requirements
		constexpr ResourceSpec ores[] = {
			{BlockId::Gold, 7.0f, 0.11f},
			{BlockId::Iron, 5.0f, 0.2f},
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
	int32 MapGenerator2::height(const glm::vec2 pos) const noexcept {
		auto octave = [&](const float32 xs, const float32 ys, const float32 off, const float32 one = 0.0f) ENGINE_INLINE {
			// TODO: use 1d simplex when you get it workign correctly
			return ys * 0.5f * (one + simplex.value(xs * (pos.x + off), 0));
		};

		float32 h = 0.0f;

		// TODO: this may be to much variation? Might make it hard to do anything in the sky. Then again we could just offset by `basis + #`
		h += octave(0.0001f, 500.0f, 1 * 999.0f, 1.0f);
		h += octave(0.005f, 50.0f, 2 * 999.0f);
		h += octave(0.05f, 5.0f, 3 * 999.0f);
		h += octave(0.75f, 2.0f, 4 * 999.0f);
		return static_cast<int32>(h);
	}*/

	auto MapGenerator2::biomeAt(const glm::vec2 pos) const noexcept -> BiomeBounds {
		for (int l = 0; l < std::size(biomeScales); ++l) {
			const glm::ivec2 cell = glm::floor((pos - biomeOffset) * biomeScalesInv[l]);
			if (perm(cell.x, cell.y) < 10) {
				//return BlockId::Debug + static_cast<BlockId>(l);
				return {l, cell};
			}
		}

		return {-1};
	}

	float32 MapGenerator2::height0(const float32 x) const noexcept {
		return heightVar * simplex.scaled(0.000005f * x, 0); // TODO: 1d simplex
	}
	
	template<MapGenerator2::Biome B>
	int32 MapGenerator2::height(const float32 x, const BiomeBounds bounds, const float32 h0) const noexcept {
		const auto hOff1 = biomeHeightOffset<Biome::Default>(x);
		auto h = hOff1;

		if constexpr (B != Biome::Default) {
			if (bounds.depth >= 0) {
				const auto hOff2 = biomeHeightOffset<B>(x);
				const auto b2s = biomeHeightStrength<B>(x, bounds);
				h = hOff1 + b2s * (hOff2 - hOff1);
			}
		}

		return static_cast<int32>(h + h0);
	}
	
	template<MapGenerator2::Biome B>
	auto MapGenerator2::landmark2(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h) const noexcept -> LandmarkSample {
		LandmarkSample res = {};

		const auto sample = [&]<Landmark L>() ENGINE_INLINE {
			res = landmarkBasis<L>(pos, ipos);
		};

		constexpr auto iter = [&]<class T, T... Is>(std::integer_sequence<T, Is...>) ENGINE_INLINE {
			((sample.template operator()<landmarksByBiome<B>[Is]>(), res.exists) || ...);
		};

		iter(std::make_index_sequence<landmarksByBiome<B>.size()>{});
		return res;
	}

	BlockId MapGenerator2::landmark(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h) const noexcept {
		// TODO: select on biome
		if (ipos.y <= h) { return BlockId::None; }

		constexpr float32 treeSpacing = 11; // Average spacing between tree centers
		constexpr int32 treeThresh = 200; // Chance for a tree to exist at this spacing out of 255
		const auto left = Engine::Noise::floorTo<int32>(ipos.x * (1.0f / treeSpacing)); // Use the left grid cell to determine trees

		if (perm.value(left) < treeThresh) {
			const int32 treeH = h + 25 + static_cast<int32>(perm.value(left - 321) * (50.0f/255.0f));
			if (ipos.y < treeH) {
				constexpr int32 trunkD = 3; // Trunk width // TODO: doesnt work correctly for even width.
				constexpr int32 trunkR = (trunkD / 2) + (trunkD / 2.0f != trunkD / 2);
				constexpr float32 pad = (0 + trunkR) * (1.0f/treeSpacing);

				// If the padding is near 0.5 you might as well use fixed spacing
				static_assert(pad < 0.45f, "Tree width is to large relative to tree spacing");

				const float32 a = left + pad;
				const float32 b = left + 1.0f - pad;
				const float32 off = perm.value(left + 321) * (1.0f/255.0f);
				const int32 wpos = Engine::Noise::floorTo<int32>((a + off * (b - a)) * treeSpacing);
				if (ipos.x < (wpos + trunkR) && ipos.x > (wpos - trunkR)) {
					return BlockId::Debug2;
				}
			}
		}
				
		return BlockId::None;
	}
	
	template<MapGenerator2::Biome B>
	float32 MapGenerator2::basisStrength(const glm::vec2 pos, const glm::vec2 posBiome, const BiomeBounds bounds) const noexcept {
		if constexpr (B != Biome::Default) {
			if (bounds.depth >= 0) {
				return biomeBasisStrength<B>(pos, posBiome, bounds);
			}
		}
		return 0.0f;
	}
	
	template<MapGenerator2::Biome B>
	float32 MapGenerator2::basis(const glm::vec2 pos, const int32 h, const float32 bstr) const noexcept {
		if constexpr (B != Biome::Default) {
			if (bstr > 0.0f) {
				const auto b2 = biomeBasis<B>(pos, h);
				if (bstr >= 1.0f) { return b2; }

				const auto b1 = biomeBasis<Biome::Default>(pos, h);
				return b1 + bstr * (b2 - b1);
			}
		}

		return biomeBasis<Biome::Default>(pos, h);
	}
	
	template<MapGenerator2::Biome B>
	BlockId MapGenerator2::block(const glm::vec2 pos, const glm::vec2 ipos, const int32 h, const float32 h0, const BiomeBounds bounds, const float32 bstr) const noexcept {
		if constexpr (B != Biome::Default) {
			if (bounds.depth >= 0 && biomeBlockStrength<B>(bstr)) {
				return biomeBlock<B>(pos, ipos, h, h0, bounds);
			}
		}

		return biomeBlock<Biome::Default>(pos, ipos, h, h0, bounds);
	}

	/*
	int32 MapGenerator2::biome(const glm::vec2 pos) const noexcept {
		// Perturb
		const auto p = simplex.value(pos.x * 0.0006f, pos.y * 0.0006f);
		const auto p2 = pos * 0.0003f + p * 0.3f;

		// Edge noise
		const auto m = simplex.value(pos.x * 0.3f, pos.y * 0.3f);

		// Main shape
		const auto wv = worley.valueF2F1(p2.x, p2.y);

		// Combine
		const auto v = wv.value - m * 0.08f;

		return v < 0.2f ? 0 : perm(wv.cell.x, wv.cell.y);
	}*/
}
