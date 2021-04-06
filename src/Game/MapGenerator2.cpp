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
		const int32 h = height(pos);
		
		if (pos.y > h) {
			constexpr float32 treeSpacing = 11; // Average spacing between tree centers
			constexpr int32 treeThresh = 200; // Chance for a tree to exist at this spacing out of 255
			const auto left = Engine::Noise::floorTo<int32>(x * (1.0f / treeSpacing)); // Use the left grid cell to determine trees

			if (treePerm.value(left) < treeThresh) {
				const int32 height = h + 25 + static_cast<int32>(treePerm.value(left - 321) * (50.0f/255.0f));
				if (pos.y < height) {
					constexpr int32 trunkD = 3; // Trunk width // TODO: doesnt work correctly for even width.
					constexpr int32 trunkR = (trunkD / 2) + (trunkD / 2.0f != trunkD / 2);
					constexpr float32 pad = (0 + trunkR) * (1.0f/treeSpacing);

					// If the padding is near 0.5 you might as well use fixed spacing
					static_assert(pad < 0.45f, "Tree width is to large relative to tree spacing");

					const float32 a = left + pad;
					const float32 b = left + 1.0f - pad;
					const float32 off = treePerm.value(left + 321) * (1.0f/255.0f);
					const int32 wpos = Engine::Noise::floorTo<int32>((a + off * (b - a)) * treeSpacing);
					if (x < (wpos + trunkR) && x > (wpos - trunkR)) {
						return BlockId::Debug2;
					}
				}
			}
			return BlockId::Air;
		} else {
			// Add grass to "top" layer
			if (y > 0) { // TODO: This doesnt really work. Think about overhangs.
				if ((height({x, y + 1}) < y + 1) ||
					(height({x + 1, y}) < y) ||
					(height({x - 1, y}) < y)) {
					return BlockId::Grass;
				}
			}

			// Add resources
			if (const auto r = resource(pos)) {
				return r;
			}

			return BlockId::Dirt;
		}
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
}
