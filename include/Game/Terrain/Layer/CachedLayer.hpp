#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	// TODO: at some point it would be convenient to move `get`, `getCacheSizeBytes`, and
	//       `cache` to this class to this base class if possible. Need to normalize and
	//       combine get/get2/get3 first.
	class CachedLayer {
		public:
			CachedLayer(TestGenerator& generator, const SeqNum& curSeq)
				: generator{generator}
				, curSeq{curSeq} {
			}

			[[nodiscard]] ENGINE_INLINE SeqNum getSeq() const noexcept { return curSeq; }

		protected:
			TestGenerator& generator;

			// This is the exact same sequence number as in generator, but is stored
			// separately due to circular dependancy jank.
			const SeqNum& curSeq;
	};
}
