#pragma once

// STD
#include <array>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Noise {
	// TODO: Doc
	template<int Count, int Mean, int Min, int Max>
	class PoissonDistribution {
		static_assert(Min < Mean);
		static_assert(Max > Mean);
		public:
			PoissonDistribution();
			int32 operator[](const int i) const;

		private:
			std::array<int8, Count> data;
	};

	const static inline auto poisson2 = PoissonDistribution<256, 2, 1, 10>{};
	const static inline auto poisson3 = PoissonDistribution<256, 3, 1, 10>{};
	const static inline auto poisson4 = PoissonDistribution<256, 4, 1, 10>{};
}

#include <Engine/Noise/PoissonDistribution.ipp>
