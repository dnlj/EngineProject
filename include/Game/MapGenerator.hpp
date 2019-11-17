#pragma once

// Engine
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/Noise/RangePermutation.hpp>

// Game
#include <Game/Common.hpp>


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
				return -1.0f;
				//return noise.value(x * scale, y * scale);
			}
		private:
			Engine::Noise::OpenSimplexNoise noise;
	};

	class BiomeB : public Biome {
		public:
			BiomeB(int64 seed) : noise{seed} {
			}

			virtual float32 value(int32 x, int32 y) override {
				constexpr float32 scale = 0.05f;
				return -0.5f;
				//return -noise.value(x * scale, y * scale);
			}
		private:
			Engine::Noise::OpenSimplexNoise noise;
	};

	class BiomeC : public Biome {
		public:
			BiomeC(int64 seed) {
			}

			virtual float32 value(int32 x, int32 y) override {
				return 0.0f;
			}
	};

	class BiomeD : public Biome {
		public:
			BiomeD(int64 seed) {
			}

			virtual float32 value(int32 x, int32 y) override {
				return 0.5f;
			}
	};

	class BiomeE : public Biome {
		public:
			BiomeE(int64 seed) {
			}

			virtual float32 value(int32 x, int32 y) override {
				return 1.0f;
			}
	};

	template<class... Biomes>
	class MapGenerator {
		public:
			MapGenerator(int64 seed)
				: biomeStorage{ Biomes{seed} ...}
				, worley{seed}
				, biome{seed} {
			}

			int32 biomeAt(int32 x, int32 y) {
				constexpr float32 scale = 0.01f;
				const auto res = worley.value(x * scale, y * scale);
				return biome.value(res.x, res.y, res.n);
			}

			float32 value(int32 x, int32 y) {
				auto& b1 = *biomes[biomeAt(x, y)];
				return b1.value(x, y);
				//return sqrt(worley.value(x * 0.01f, y * 0.01f).distanceSquared);
			}

		private:
			std::tuple<Biomes...> biomeStorage;
			Biome* biomes[sizeof...(Biomes)] = { &std::get<Biomes>(biomeStorage) ... };
			Engine::Noise::WorleyNoiseFrom<&Engine::Noise::constant1> worley;
			Engine::Noise::RangePermutation<sizeof...(Biomes)> biome;
	};
}
