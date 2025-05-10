#pragma once


namespace Game::Terrain {
	template<class T>
	class ChunkStore {
		private:
			T store[chunkSize.x][chunkSize.y]{};

		public:
			ChunkStore() = default;
			ChunkStore(ChunkStore&&) = default;
			ChunkStore(const ChunkStore&) = delete;

			ENGINE_INLINE_REL T& at(ChunkVec chunkIndex) noexcept {
				ENGINE_DEBUG_ASSERT((chunkIndex.x >= 0) && (chunkIndex.x < chunkSize.x), "Attempting to access block outside of ChunkStore.");
				ENGINE_DEBUG_ASSERT((chunkIndex.y >= 0) && (chunkIndex.y < chunkSize.y), "Attempting to access block outside of ChunkStore.");
				return store[chunkIndex.x][chunkIndex.y];
			}

			ENGINE_INLINE_REL const T& at(ChunkVec chunkIndex) const noexcept {
				return const_cast<ChunkStore*>(this)->at(chunkIndex);
			}
	};
}
