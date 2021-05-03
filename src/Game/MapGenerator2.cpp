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

#define DEF_BIOME_HEIGHT_OFFSET(I)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeHeightOffset<I>(const float32 x) const noexcept

#define DEF_BIOME_HEIGHT_STRENGTH(I)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeHeightStrength<I>(const float32 x, const BiomeBounds bounds) const noexcept

#define DEF_BIOME_BASIS(I)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeBasis<I>(const BiomeParams_Basis& params) const noexcept

#define DEF_BIOME_BASIS_STRENGTH(I)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeBasisStrength<I>(const BiomeParams_BasisStrength& params) const noexcept

#define DEF_BIOME_BLOCK(I)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE BlockId MapGenerator2::biomeBlock<I>(const BiomeParams_Block& params) const noexcept

#define DEF_BIOME_BLOCK_STRENGTH(I)\
	template<>\
	[[nodiscard]]\
	ENGINE_INLINE float32 MapGenerator2::biomeBlockStrength<I>(const BiomeParams_BlockStrength& params) const noexcept

////////////////////////////////////////////////////////////////////////////////
// Biome 0
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(0) {
		return 15 * simplex.scaled(x * 0.05f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(0) {
		return 1.0f;
	}

	DEF_BIOME_BASIS(0) {
		if (params.pos.y > params.h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr float32 groundScale = 1.0f / 100.0f;
		const float32 groundGrad = std::max(0.0f, 1.0f - (params.h - params.pos.y) * groundScale);
		return simplex.scaled(params.pos * 0.06f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(0) {
		return 1;
	}

	DEF_BIOME_BLOCK(0) {
		// Add grass to "top" layer
		if (params.pos.y > -heightVar) {
			// TODO: integer pos should be passed when we get stage data working
			// TODO: enable
			//if ((h == params.ipos.y) || (height(pos.x - 1, bounds) < params.ipos.y) || (height(pos.x + 1, bounds) < params.ipos.y)) {
			//	return BlockId::Grass;
			//}
		}

		if (const auto r = resource(params.pos)) {
			return r;
		}

		return BlockId::Dirt;
	}

	DEF_BIOME_BLOCK_STRENGTH(0) {
		return 1.0f;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Biome 1
////////////////////////////////////////////////////////////////////////////////
namespace Game {
	DEF_BIOME_HEIGHT_OFFSET(1) {
		return 85 * simplex.scaled(x * 0.01f, 0); // TODO: 1d simplex
	}

	DEF_BIOME_HEIGHT_STRENGTH(1) {
		// TODO: we use this radius/offset type logic in a number of places. Probably extract and pass as args.
		const auto s = biomeScales[bounds.depth];
		const auto center = (bounds.cell.x + 0.5f) * s;
		const auto off = 0.5f * s - std::abs(center - x + biomeOffset.x);
		constexpr auto tDist = 1.0f / 350.0f; // 1 / transition distance in blocks
		return std::min(1.0f, off * tDist);
	}

	DEF_BIOME_BASIS(1) {
		if (params.pos.y > params.h) { return -1; }

		// TODO: we really want this to be a very large gradient so caves get larger with depth
		constexpr float32 groundScale = 1.0f / 100.0f;
		const float32 groundGrad = std::max(0.0f, 1.0f - (params.h - params.pos.y) * groundScale);
		return simplex.scaled(params.pos * 0.01f) + groundGrad;
	}

	DEF_BIOME_BASIS_STRENGTH(1) {
		const auto s = biomeScales[params.bounds.depth];
		const auto center = (glm::vec2{params.bounds.cell} + 0.5f) * s;
		const auto off = 0.5f * s - glm::abs(center - params.posBiome + biomeOffset);
		constexpr auto tDist = 1.0f / 350.0f; // 1 / transition distance in blocks
		return glm::compMin(glm::min(off * tDist, 1.0f));
	}

	DEF_BIOME_BLOCK(1) {
		return BlockId::Debug + (BlockId)params.bounds.depth;
	}

	DEF_BIOME_BLOCK_STRENGTH(1) {
		return 0.5f < params.basisStrength;
	}
}

#undef DEF_BIOME_HEIGHT_OFFSET
#undef DEF_BIOME_HEIGHT_STRENGTH
#undef DEF_BIOME_BASIS
#undef DEF_BIOME_BASIS_STRENGTH
#undef DEF_BIOME_BLOCK
#undef DEF_BIOME_BLOCK_STRENGTH

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
		const glm::vec2 pos = {static_cast<float32>(x), static_cast<float32>(y)};

		/*
		struct BiomeSample {
			float32 strength; // [0, 1]
			float32 basis; // ~[-1, 1]
			BlockId block; // block enum
		};

		// TODO: this is strength not basis
		/*auto biome2_strength = [&](const glm::vec2 p0, const glm::vec2 shift) -> float32 {
			// Controls how warped the shape looks
			const float32 ppv = 10.0f;
			const float32 pps = 0.1f;
			const auto ppi = ppv * p0;
			const auto ppr = pps * glm::vec2{simplex.scaled(ppi), simplex.scaled(-ppi)};

			// offset from center
			const auto off = 2.0f * p0 - 1.0f;
			const auto nOff = glm::normalize(off + ppr) + shift;

			// We want to rescale before adjustments to avoid issues around zero/center
			auto v = (1.0f - glm::length(off)) * (1.0f / 0.5f); // TODO: length2

			// Controls width(f) and depth(a) of tendrils
			constexpr float32 f1 = 4.0f;
			constexpr float32 a1 = 0.2f;
			constexpr float32 f2 = f1 * 5;
			constexpr float32 a2 = a1 / 2;

			v += a1 * simplex.scaled(f1 * nOff);
			v += a2 * simplex.scaled(f2 * nOff);

			// Controls scale(s) and size(n) of dithering
			constexpr float32 s1 = 0.07f;
			constexpr float32 n1 = 0.09f;
			constexpr float32 s2 = 0.3f;
			constexpr float32 n2 = 0.09f;

			v += n1 * simplex.scaled(s1 * pos);
			//v += n2 * simplex.scaled(s2 * pos);

			// Rescale output
			constexpr auto zero = a1 + a2 + n1;
			return v - zero;
		};

		auto biome2_basis = [&](glm::vec2 p0) -> float32 {
			//return simplex.scaled(p0 * 0.04f);
			return 1.0f;
		};*/
		/*
		auto biome2 = [&]() -> BiomeSample {
			const auto sz = biomeScalesInv[biome.depth];
			const auto cellf = glm::vec2{biome.cell};
			//const auto rel = cellf * sz + biomeOffset - pos;
			const auto rel = (pos - biomeOffset) * sz - cellf;
			ENGINE_ASSERT(glm::length(rel) <= Engine::Sqrt2<float32>); // TODO: shouldnt this be sqrt(2)?

			// TODO: need to incorporate biome.depth or else we get the same biome layouts for each depth, just at different scales
			//const auto s = biome1_strength(rel, cellf);

			return {
				.strength = biome2_strength(rel, cellf),
				.basis = biome2_basis(pos),
				.block = BlockId::Debug + static_cast<BlockId>(biome.depth),
			};
		};

		auto biome1 = [&]() -> BiomeSample {
			const int32 h = height(pos);
			if (pos.y > h) {
				constexpr float32 treeSpacing = 11; // Average spacing between tree centers
				constexpr int32 treeThresh = 200; // Chance for a tree to exist at this spacing out of 255
				const auto left = Engine::Noise::floorTo<int32>(x * (1.0f / treeSpacing)); // Use the left grid cell to determine trees

				if (perm.value(left) < treeThresh) {
					const int32 height = h + 25 + static_cast<int32>(perm.value(left - 321) * (50.0f/255.0f));
					if (pos.y < height) {
						constexpr int32 trunkD = 3; // Trunk width // TODO: doesnt work correctly for even width.
						constexpr int32 trunkR = (trunkD / 2) + (trunkD / 2.0f != trunkD / 2);
						constexpr float32 pad = (0 + trunkR) * (1.0f/treeSpacing);

						// If the padding is near 0.5 you might as well use fixed spacing
						static_assert(pad < 0.45f, "Tree width is to large relative to tree spacing");

						const float32 a = left + pad;
						const float32 b = left + 1.0f - pad;
						const float32 off = perm.value(left + 321) * (1.0f/255.0f);
						const int32 wpos = Engine::Noise::floorTo<int32>((a + off * (b - a)) * treeSpacing);
						if (x < (wpos + trunkR) && x > (wpos - trunkR)) {
							return {
								.strength = 1.0f,
								.basis = 0.0f,
								.block = BlockId::Debug2,
							};
						}
					}
				}
				
				return {
					.strength = 1.0f,
					.basis = -1.0f,
					.block = BlockId::Air,
				};
			} else {
				// Add grass to "top" layer
				if (y > 0) { // TODO: This doesnt really work. Think about overhangs.
					if ((height({x, y + 1}) < y + 1) ||
						(height({x + 1, y}) < y) ||
						(height({x - 1, y}) < y)) {
						return {
							.strength = 1.0f,
							.basis = 1.0f,
							.block = BlockId::Grass,
						};
					}
				}

				// TODO: we really want this to be a very large gradient so caves get larger with depth
				constexpr float32 groundScale = 1.0f / 100.0f;
				const float32 groundGrad = std::max(0.0f, 1.0f - (h - pos.y) * groundScale);
				const auto v = simplex.scaled(pos * 0.06f) + groundGrad;


				// Add resources
				if (const auto r = resource(pos)) {
					return {
						.strength = 1.0f,
						.basis = v,
						.block = r,
					};
				}

				return {
					.strength = 1.0f,
					.basis = v,
					.block = BlockId::Dirt,
				};
			}
		};
		*/
		/*const auto a = biome1();
		auto s = a;

		if (biome.depth != -1) {
			const auto b = biome2();
			if (b.strength > 0) {
				//ENGINE_DEBUG_ASSERT(b.strength < 1.2f);
				//ENGINE_LOG(b.strength);

				if (b.strength > 0.99f) {
					//ENGINE_LOG(b.strength);
				}

				const auto p = b.strength < 0 ? 0 : (b.strength > 1 ? 1 : b.strength);
				s.basis = a.basis + p * (b.basis - a.basis);
				s.block = b.block;
			}
		}

		if (s.basis < 0) {
			return BlockId::Air;
		}

		return s.block;*/

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		BiomeParams params;
		params.ipos = {x, y};
		params.pos = {static_cast<float32>(x), static_cast<float32>(y)};

		// Underlying height variation in terrain
		//params.h0 = height0(static_cast<BiomeParams_Height0&>(params));
		params.h0 = height0(params);
		params.posBiome = pos - glm::vec2{0, params.h0};
		params.bounds = biomeAt(params.posBiome); // TODO: rn - bounds

		/////////////////////////////////////////////////////////////////////////////
		params.h = height(params);

		auto basis = [&]{
			if (params.bounds.depth >= 0) {
				params.basisStrength = biomeBasisStrength<1>(params);
				const auto b2 = biomeBasis<1>(params);
				if (params.basisStrength >= 1.0f) { return b2; }

				const auto b1 = biomeBasis<0>(params);
				return b1 + params.basisStrength * (b2 - b1);
			}

			return biomeBasis<0>(params);
		};

		if (basis() <= 0) {
			return BlockId::Air;
		};

		auto biome2_blockStrength = [&]() -> bool {
			return 0.5f < params.basisStrength;
		};

		auto block = [&]{
			if (params.bounds.depth >= 0 && biomeBlockStrength<1>(params)) {
				return biomeBlock<1>(params);
			} else {
				return biomeBlock<0>(params);
			}
		};

		return block();
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

	float32 MapGenerator2::height0(const BiomeParams_Height0& params) const noexcept {
		return heightVar * simplex.scaled(0.000005f * params.pos.x, 0); // TODO: 1d simplex
	}

	int32 MapGenerator2::height(const BiomeParams_Height& params) const noexcept {
		const auto hOff1 = biomeHeightOffset<0>(params.pos.x);
		auto h = hOff1;

		if (params.bounds.depth >= 0) {
			const auto hOff2 = biomeHeightOffset<1>(params.pos.x);
			const auto b2s = biomeHeightStrength<1>(params.pos.x, params.bounds);
			h = hOff1 + b2s * (hOff2 - hOff1);
		}

		return static_cast<int32>(h + height0(params));
	}

	auto MapGenerator2::biomeAt(const glm::vec2 pos) const noexcept -> BiomeBounds {
		for (int l = 0; l < std::size(biomeScales); ++l) {
			const glm::ivec2 cell = glm::floor((pos - biomeOffset) * biomeScalesInv[l]);
			if (perm(cell.x, cell.y) < 10) {
				//return BlockId::Debug + static_cast<BlockId>(l);
				return {l, cell};
			}
		}

		//return BlockId::None;
		return {-1};
	}

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
	}
}
