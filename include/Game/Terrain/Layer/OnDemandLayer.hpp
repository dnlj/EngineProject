#pragma once

namespace Game::Terrain::Layer {
	/**
	 * Base class for layers that don't cache generation results. This is usually because
	 * they are either extremely cheap or are use only once and then cached by another
	 * system (biome layers).
	 */
	class OnDemandLayer {
		public:
			OnDemandLayer() = default;
			OnDemandLayer(TestGenerator& generator, const SeqNum& curSeq) {}
			ENGINE_INLINE void partition(const auto&...) {};
			ENGINE_INLINE void generate(const auto&...) {};
			ENGINE_INLINE uint64 getCacheSizeBytes() const noexcept { return 0; }
			ENGINE_INLINE void clearCache(SeqNum minAge) const noexcept {}
	};
}
