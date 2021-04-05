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
				if (chunk.data[x][y] > BlockId::Air && (x == 0 || y == 0)) {
					chunk.data[x][y] = BlockId::Debug;
				}
			}
		}

		chunk.pass = Pass::Basis;
	}

	void MapGenerator2::advance(const glm::ivec2 pos, MapChunk& chunk, const MapChunk& top, const MapChunk& right, const MapChunk& bottom, const MapChunk& left) {
		switch (chunk.pass) {
			case Pass::Basis: { return; }
			case Pass::Foliage: { return; }
			default: {
				ENGINE_WARN("Unknown chunk pass ", static_cast<int>(chunk.pass));
				return;
			}
		}
	}

	BlockId MapGenerator2::value(const int32 x, const int32 y) const noexcept {
		const glm::vec2 pos = {static_cast<float32>(x), static_cast<float32>(y)};
		float32 v = basis(pos);

		// Cave size
		const float32 s = gradient(pos.y, {
			// TODO:
			{ -100.0f,  0.25f},
			{-1000.0f,  0.25f},
		});

		if (v < s) {
			return BlockId::Air;
		} else {
			// Add grass to "top" layer
			if (pos.y > 0) { // TODO: This doesnt really work. Think about overhangs.
				if ((basis({pos.x, pos.y + 1}) < s) ||
					(basis({pos.x + 1, pos.y}) < s) ||
					(basis({pos.x - 1, pos.y}) < s)) {
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
	
	float32 MapGenerator2::basis(const glm::vec2 pos) const noexcept {
		float32 v = 0.0f;

		if (pos.y > 0) {
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

			v = pos.y < h;
		} else {
			v = 1.0f;
		}

		// TODO: caves

		// Ground gradient
		//v += gradient(pos.y, {
		//	{ 25.0f, -3.0f},
		//	{  0.0f,  3.0f},
		//	{-50.0f,  0.0f},
		//});

		return v;
	}
}
