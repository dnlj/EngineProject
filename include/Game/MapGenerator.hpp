#pragma once

// Engine
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/Noise/RangePermutation.hpp>

// Game
#include <Game/Common.hpp>


// TODO: split
namespace Game {
	// TODO: move
	class Biome {
		public:
			virtual float32 value(int32 x, int32 y) = 0;
	};

	class BiomeA : public Biome {
		public:
			BiomeA(int64 seed) : noise{seed} {
			}

			virtual float32 value(int32 x, int32 y) override {
				constexpr float32 scale = 0.05f;
				return -1;
				return noise.value(x * scale, y * scale);
			}
		private:
			Engine::Noise::OpenSimplexNoise noise;
	};

	class BiomeB : public Biome {
		public:
			BiomeB(int64 seed) : noise{seed} {
			}

			virtual float32 value(int32 x, int32 y) override {
				constexpr float32 scale = 0.03f;
				return noise.value(x * scale, y * scale);
			}
		private:
			Engine::Noise::OpenSimplexNoise noise;
	};

	class BiomeC : public Biome {
		public:
			BiomeC(int64 seed) : noise{seed} {
			}

			virtual float32 value(int32 x, int32 y) override {
				constexpr float32 scale = 0.01f;
				return 1;
				return noise.value(x * scale, y * scale);
			}
		private:
			Engine::Noise::OpenSimplexNoise noise;
	};

	template<class... Biomes>
	class MapGenerator {
		public:
			MapGenerator(const int64 seed)
				: biomeStorage{ Biomes{seed} ...}
				, worley{seed}
				, biome{seed} {
			}

			float32 value(const int32 x, const int32 y) {
				float32 weights[BIOME_COUNT] = {};
				float32 total = 0;
				{
					const auto biome = biomeAt(x, y);
					weights[biome.index] += biome.strength;
					total += biome.strength;
				}

				constexpr int32 steps = 4;
				for (int32 i = 0; i < steps; ++i) {
					constexpr float32 step = 2 * Engine::PI / steps;
					constexpr int32 r = 16;
					const float32 ang = step * i;
					const int32 xi = x + Engine::Noise::floorTo<int32>(r * cos(ang));
					const int32 yi = y + Engine::Noise::floorTo<int32>(r * sin(ang));
					const auto biome = biomeAt(xi, yi);
					weights[biome.index] += biome.strength;
					total += biome.strength;
				}

				float32 avg = 0;
				for (int32 biome = 0; biome < BIOME_COUNT; ++biome) {
					float32 w = weights[biome];
					if (w == 0) { continue; }

					float32 relW = w / total;
					avg += biomes[biome]->value(x, y) * relW;
				}

				return avg;
			}

		private:
			constexpr static auto BIOME_COUNT = sizeof...(Biomes);
			static_assert(BIOME_COUNT > 0, "Cannot generate a map without any biomes");

			std::tuple<Biomes...> biomeStorage;
			Biome* biomes[BIOME_COUNT] = { &std::get<Biomes>(biomeStorage) ... };
			Engine::Noise::WorleyNoiseFrom<&Engine::Noise::constant1> worley; // TODO: if we just used packed circles on a grid we would probably get similar results much faster. Consider?
			Engine::Noise::RangePermutation<BIOME_COUNT> biome;

			class BiomeValue {
				public:
					int32 index;
					float32 strength;
			};

			BiomeValue biomeAt(const int32 x, const int32 y) {
				constexpr float32 scale = 0.005f;
				const auto res = worley.valueF2F1(x * scale, y * scale);
				return {biome.value(res.x, res.y, res.n), res.value};
			}
	};
}
