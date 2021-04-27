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

		const auto b = biome2(pos);
		//if (b && b < 40) { return BlockId::Debug2; }
		//if (b != BlockId::None) { return b; }
		//if (b.depth >= 0) {
		//	return BlockId::Debug + static_cast<BlockId>(b.depth);
		//}

		struct BiomeSample {
			float32 basis;
			BlockId block;
		};

		auto biome2 = [&](const glm::vec2 p0, const glm::vec2 shift) -> BiomeSample {
			// Controls how warped the shape looks
			const float32 ppv = 10.0f;
			const float32 pps = 0.1f;
			const auto ppi = ppv * p0;
			const auto ppr = pps * glm::vec2{simplex.scaled(ppi), simplex.scaled(-ppi)};

			// offset from center [-pps, pps]
			const auto off = 2.0f * p0 + 1.0f + ppr;
			const auto nOff = glm::normalize(off) + shift;
			auto v = glm::length(off);

			// Controls width(f) and depth(a) of tendrils
			constexpr float32 f1 = 4.0f;
			constexpr float32 a1 = 0.2f;
			constexpr float32 f2 = f1 * 5;
			constexpr float32 a2 = a1 / 2;

			v += a1 * simplex.scaled(f1 * nOff);
			v += a2 * simplex.scaled(f2 * nOff);

			// Controls scale(s) and size(n) of dithering
			constexpr float32 s1 = 0.02f;
			constexpr float32 n1 = 0.07f;
			constexpr float32 s2 = 0.3f;
			constexpr float32 n2 = 0.09f;

			// TODO: could probably get away with only one layer of a in-between size/strength
			v += n1 * simplex.scaled(s1 * pos);
			v += n2 * simplex.scaled(s2 * pos);

			// Rescale output
			constexpr auto max = 1.0f - a1 - a2 - n1 - n2;
			static_assert(max > 0);

			if (v < max) {
				constexpr auto m = 1.0f / max;
				//return 1.0f - v * m;
				return {
					.basis = 1.0f - v * m,
					.block = BlockId::Debug + static_cast<BlockId>(b.depth),
				};
			}

			return {
				.basis = 0.0f,
				.block = BlockId::Air,
			};
		};

		{
			const auto sz = biomeScales[b.depth];
			const auto cellf = glm::vec2{b.cell};
			const auto rel = cellf * sz + biomeOffset - pos;
			const auto r = biome2(rel / sz, cellf);
			if (r.basis > 0) {
				return r.block;
			}
		}

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
								.basis = 0.0f,
								.block = BlockId::Debug2,
							};
						}
					}
				}
				
				return {
					.basis = 0.0f,
					.block = BlockId::Air,
				};
			} else {
				// Add grass to "top" layer
				if (y > 0) { // TODO: This doesnt really work. Think about overhangs.
					if ((height({x, y + 1}) < y + 1) ||
						(height({x + 1, y}) < y) ||
						(height({x - 1, y}) < y)) {
						return {
							.basis = 1.0f,
							.block = BlockId::Grass,
						};
					}
				}

				// Add resources
				if (const auto r = resource(pos)) {
					return {
						.basis = 1.0f,
						.block = r,
					};
				}

				return {
					.basis = 1.0f,
					.block = BlockId::Dirt,
				};
			}
		};

		const auto base = biome1();
		return base.block;
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
	}

	auto MapGenerator2::biome2(const glm::vec2 pos) const noexcept -> BiomeBounds {
		constexpr auto scales = []{
			std::array<float32, std::size(biomeScales)> inv;
			for (int i = 0; const auto s : biomeScales) {
				inv[i++] = 1.0f / s;
			}
			return inv;
		}();

		for (int l = 0; l < std::size(scales); ++l) {
			const glm::ivec2 cell = glm::floor((pos - biomeOffset) * scales[l]);
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
