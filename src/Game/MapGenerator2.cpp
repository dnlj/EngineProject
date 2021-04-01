#include <Game/MapGenerator2.hpp>

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
	BlockId MapGenerator2::value(const int32 x, const int32 y) const noexcept {
		const glm::vec2 pos = {static_cast<float32>(x), static_cast<float32>(y)};
		float32 v = basis(pos);

		// Cave size
		const float32 s = gradient(pos.y, {
			{ -50.0f,  0.0f},
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
			} else if (const auto r = resource(pos)) {
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

		{ // Main caves
			auto p = pos;
			p *= 0.05f;
			v += simplex.value(p.x, p.y);

			p *= 2.0f;
			v += simplex.value(p.x + 9999.0f, p.y + 9999.0f);
		
			p *= 1.25f;
			v += simplex.value(p.x - 9999.0f, p.y - 9999.0f);
		}

		// Ground gradient
		v += gradient(pos.y, {
			{ 25.0f, -3.0f},
			{  0.0f,  3.0f},
			{-50.0f,  0.0f},
		});

		return v;
	}
}
