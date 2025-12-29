#pragma once
#if false
// Game
#include <Game/Terrain/ChunkArea.hpp>

// Engine
#include <Engine/BaseMember.hpp>


#pragma warning(push)
#pragma warning(disable : 4584) // Duplicate base class: BaseMember<Engine::None>

// TODO: Should probably be in Engine.

// TODO: Do we need extra walkers for index vs coord walkers?

// TODO: Should we remove the Y from the names and instead just use the unqualified name?
//       That might make it a bit more obvious how the walkers are used. Although the Y
//       naming is accurate and pairs nicely with the X naming. I'm referring to names such
//       as `onNextChunkY(...)` vs just `onNextChunk(...)`
namespace Game::Terrain {
	class AreaWalkRegionParam {
		public:
			RegionVec regionCoord;
	};

	class AreaWalkChunkParam : public AreaWalkRegionParam {
		public:
			RegionVec regionIndex;
			ChunkVec chunkCoord;
	};

	class AreaWalkBlockParam : public AreaWalkChunkParam {
		public:
			ChunkVec chunkIndex;
			BlockVec blockCoord;
	};

	class AreaWalkParam : public AreaWalkBlockParam {
	};

	namespace Detail {
		////////////////////////////////////////////////////////////////////////////////

		// TODO: This isn't good... would need to be more advanced. Doesn't do the
		//       isPopulated skipping for already populated areas that the region cache
		//       does. We could add a return value to the callbacks?
		/**
		 * This serves as the leaf node for walker inheritance. This is where we can
		 * define how we walk different types/areas/stores/caches.
		 */
		template<class Unused, class ParentGet>
		class ENGINE_EMPTY_BASE AreaWalker : public ParentGet {
			public:
				// Unused is here to allow all the walkers to be uniformly defined. It will always be Engine::None.
				static_assert(std::same_as<Unused, Engine::None>);

				constexpr AreaWalker(Unused&& unused, ParentGet&& parent) : ParentGet{std::move(parent)} {}
				constexpr void walk(ChunkArea area) {
					// TODO: Specialize, if no region walkers are given then we can ignore
					//       regions and just iterate chunks. How common is that use case
					//       though? I think right now we frequently end up going chunk >
					//       region internally in our manual loops anyways. If that's the
					//       case then this would still be a more efficient option.
					//
					//       Same for blocks. If we don't specify region or chunk walkers we
					//       can just iterate blocks directly unless we need the index for some reason.

					const auto regionCoordMin = chunkToRegion(area.min);
					const auto regionCoordMax = chunkToRegion(area.max);
					const auto regionCoordLast = regionCoordMax - RegionVec{1, 1};

					const auto regionIndexMin = chunkToRegionIndex(area.min, regionCoordMin);
					const auto regionIndexMax = chunkToRegionIndex(area.max, regionCoordMax);
					auto regionIndexLowerLimit = regionIndexMin;
					auto regionIndexUpperLimit = regionSize;

					// Per region
					AreaWalkParam param{};
					for (param.regionCoord = regionCoordMin; param.regionCoord.x < regionCoordMax.x; ++param.regionCoord.x) {
						// Check if we need to limit the upper index x within the region.
						if (param.regionCoord.x == regionCoordMax.x - 1) {
							regionIndexUpperLimit.x = regionIndexMax.x;
						}

						if constexpr (requires { this->getRegionWalkerX(); }) {
							this->getRegionWalkerX()(std::as_const(param));
						}

						for (param.regionCoord.y = regionCoordMin.y; param.regionCoord.y < regionCoordMax.y; ++param.regionCoord.y) {
							// Check if we need to limit the upper index y within the region.
							if (param.regionCoord.y == regionCoordMax.y - 1) {
								regionIndexUpperLimit.y = regionIndexMax.y;
							}

							if constexpr (requires { this->getRegionWalkerY(); }) {
								this->getRegionWalkerY()(std::as_const(param));
							}

							// Every chunk in this region between the upper and lower limits.
							param.regionIndex = regionIndexLowerLimit;
							const auto regionChunkCoord = regionToChunk(param.regionCoord);
							for (; param.regionIndex.x < regionIndexUpperLimit.x; ++param.regionIndex.x) {
								param.regionIndex.y = regionIndexLowerLimit.y;
								param.chunkCoord = regionChunkCoord + param.regionIndex;

								if constexpr (requires { this->getChunkWalkerX(); }) {
									this->getChunkWalkerX()(std::as_const(param));
								}

								for (; param.regionIndex.y < regionIndexUpperLimit.y; ++param.regionIndex.y) {
									param.chunkCoord.y = param.regionCoord.y + param.regionIndex.y;
									if constexpr (requires { this->getChunkWalkerY(); }) {
										this->getChunkWalkerY()(std::as_const(param));
									}

									// For every block in the chunk
									param.chunkIndex = {};
									const auto chunkBlockCoord = chunkToBlock(param.chunkCoord);
									for (; param.chunkIndex.x < chunkSize.x; ++param.chunkIndex.x) {
										param.blockCoord = chunkBlockCoord + param.chunkIndex;
										if constexpr (requires { this->getChunkWalkerY(); }) {
											this->getBlockWalkerX()(std::as_const(param));
										}

										for (; param.chunkIndex.y < chunkSize.y; ++param.chunkIndex.y) {
											param.blockCoord.y = chunkBlockCoord.y + param.chunkIndex.y;
											if constexpr (requires { this->getChunkWalkerY(); }) {
												this->getBlockWalkerY()(std::as_const(param));
											}
										}
									}
								}
							}

							// We won't visit the y lower limit again until we advance the region in x.
							regionIndexLowerLimit.y = 0;
						}

						// We will never visit the x lower limit again. Start over at the y lower limit.
						regionIndexLowerLimit.x = 0;
						regionIndexLowerLimit.y = regionIndexMin.y;
						regionIndexUpperLimit.y = regionSize.y;
					}
				};
		};

		////////////////////////////////////////////////////////////////////////////////
	
		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaBlockWalkerYGet : private Engine::BaseMember<Func>, public ParentGet {
			public:
				ENGINE_INLINE constexpr AreaBlockWalkerYGet(Func&& func, ParentGet&& parent)
					: Engine::BaseMember<Func>{std::move(func)}
					, ParentGet{std::move(parent)} {
				}
			
				ENGINE_INLINE constexpr Func& getBlockWalkerY() noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}

				ENGINE_INLINE constexpr const Func& getBlockWalkerY() const noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}
		};

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaBlockWalkerY : public AreaWalker<Engine::None, AreaBlockWalkerYGet<Func, ParentGet>> {
			public:
				using BaseWalk = AreaWalker<Engine::None, AreaBlockWalkerYGet<Func, ParentGet>>;
				using BaseGet = AreaBlockWalkerYGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaBlockWalkerY(Func&& func, ParentGet&& parent)
					: BaseWalk{Engine::None{}, BaseGet{std::move(func), std::move(parent)}} {
				}
		};

		////////////////////////////////////////////////////////////////////////////////
	
		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaBlockWalkerXGet : private Engine::BaseMember<Func>, public ParentGet {
			public:
				ENGINE_INLINE constexpr AreaBlockWalkerXGet(Func&& func, ParentGet&& parent)
					: Engine::BaseMember<Func>{std::move(func)}
					, ParentGet{std::move(parent)} {
				}
			
				ENGINE_INLINE constexpr Func& getBlockWalkerX() noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}

				ENGINE_INLINE constexpr const Func& getBlockWalkerX() const noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}
		};

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaBlockWalkerX : public AreaBlockWalkerY<Engine::None, AreaBlockWalkerXGet<Func, ParentGet>> {
			public:
				using BaseWalk = AreaBlockWalkerY<Engine::None, AreaBlockWalkerXGet<Func, ParentGet>>;
				using BaseGet = AreaBlockWalkerXGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaBlockWalkerX(Func&& func, ParentGet&& parent)
					: BaseWalk{Engine::None{}, BaseGet{std::move(func), std::move(parent)}} {
				}

				// TODO: move *this instead of copy.
				template<class Next>
				ENGINE_INLINE constexpr AreaBlockWalkerY<Next, BaseGet> onNextBlockY(Next&& next) const noexcept {
					return {std::forward<Next>(next), BaseGet{*this}};
				}
		};

		////////////////////////////////////////////////////////////////////////////////
	
		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaChunkWalkerYGet : private Engine::BaseMember<Func>, public ParentGet {
			public:
				ENGINE_INLINE constexpr AreaChunkWalkerYGet(Func&& func, ParentGet&& parent)
					: Engine::BaseMember<Func>{std::move(func)}
					, ParentGet{std::move(parent)} {
				}
			
				ENGINE_INLINE constexpr Func& getChunkWalkerY() noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}

				ENGINE_INLINE constexpr const Func& getChunkWalkerY() const noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}
		};

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaChunkWalkerY : public AreaBlockWalkerX<Engine::None, AreaChunkWalkerYGet<Func, ParentGet>> {
			public:
				using BaseWalk = AreaBlockWalkerX<Engine::None, AreaChunkWalkerYGet<Func, ParentGet>>;
				using BaseGet = AreaChunkWalkerYGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaChunkWalkerY(Func&& func, ParentGet&& parent)
					: BaseWalk{Engine::None{}, BaseGet{std::move(func), std::move(parent)}} {
				}

				template<class Next>
				ENGINE_INLINE constexpr AreaBlockWalkerX<Next, BaseGet> onNextBlockX(Next&& next) const noexcept {
					return {std::forward<Next>(next), BaseGet{*this}};
				}
		};

		////////////////////////////////////////////////////////////////////////////////

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaChunkWalkerXGet : private Engine::BaseMember<Func>, public ParentGet {
			public:
				ENGINE_INLINE constexpr AreaChunkWalkerXGet(Func&& func, ParentGet&& parent)
					: Engine::BaseMember<Func>{std::move(func)}
					, ParentGet{std::move(parent)} {
				}
			
				ENGINE_INLINE constexpr Func& getChunkWalkerX() noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}

				ENGINE_INLINE constexpr const Func& getChunkWalkerX() const noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}
		};

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaChunkWalkerX : public AreaChunkWalkerY<Engine::None, AreaChunkWalkerXGet<Func, ParentGet>> {
			public:
				using BaseWalk = AreaChunkWalkerY<Engine::None, AreaChunkWalkerXGet<Func, ParentGet>>;
				using BaseGet = AreaChunkWalkerXGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaChunkWalkerX(Func&& func, ParentGet&& parent)
					: BaseWalk{Engine::None{}, BaseGet{std::move(func), std::move(parent)}} {
				}

				template<class Next>
				ENGINE_INLINE constexpr AreaChunkWalkerY<Next, BaseGet> onNextChunkY(Next&& next) const noexcept {
					return {std::forward<Next>(next), BaseGet{*this}};
				}
		};

		////////////////////////////////////////////////////////////////////////////////

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaRegionWalkerYGet : private Engine::BaseMember<Func>, public ParentGet {
			public:
				ENGINE_INLINE constexpr AreaRegionWalkerYGet(Func&& func, ParentGet&& parent)
					: Engine::BaseMember<Func>{std::move(func)}
					, ParentGet{std::move(parent)} {
				}
			
				ENGINE_INLINE constexpr Func& getRegionWalkerY() noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}

				ENGINE_INLINE constexpr const Func& getRegionWalkerY() const noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}
		};

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaRegionWalkerY : public AreaChunkWalkerX<Engine::None, AreaRegionWalkerYGet<Func, ParentGet>> {
			public:
				using BaseWalk = AreaChunkWalkerX<Engine::None, AreaRegionWalkerYGet<Func, ParentGet>>;
				using BaseGet = AreaRegionWalkerYGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaRegionWalkerY(Func&& func, ParentGet&& parent)
					: BaseWalk{Engine::None{}, BaseGet{std::move(func), std::move(parent)}} {
				}

				template<class Next>
				ENGINE_INLINE constexpr AreaChunkWalkerX<Next, BaseGet> onNextChunkX(Next&& next) const noexcept {
					return {std::forward<Next>(next), BaseGet{*this}};
				}
		};

		////////////////////////////////////////////////////////////////////////////////

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaRegionWalkerXGet : private Engine::BaseMember<Func>, public ParentGet {
			public:
				ENGINE_INLINE constexpr AreaRegionWalkerXGet(Func&& func, ParentGet&& parent)
					: Engine::BaseMember<Func>{std::move(func)}
					, ParentGet{std::move(parent)} {
				}
			
				ENGINE_INLINE constexpr Func& getRegionWalkerX() noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}

				ENGINE_INLINE constexpr const Func& getRegionWalkerX() const noexcept requires !std::same_as<Func, Engine::None> {
					return Engine::BaseMember<Func>::get();
				}
		};

		template<class Func, class ParentGet>
		class ENGINE_EMPTY_BASE AreaRegionWalkerX : public AreaRegionWalkerY<Engine::None, AreaRegionWalkerXGet<Func, ParentGet>> {
			public:
				using BaseWalk = AreaRegionWalkerY<Engine::None, AreaRegionWalkerXGet<Func, ParentGet>>;
				using BaseGet = AreaRegionWalkerXGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaRegionWalkerX(Func&& func, ParentGet&& parent)
					: BaseWalk{Engine::None{}, BaseGet{std::move(func), std::move(parent)}} {
				}

				template<class Next>
				ENGINE_INLINE constexpr AreaRegionWalkerY<Next, BaseGet> onNextRegionY(Next&& next) const noexcept {
					return {std::forward<Next>(next), BaseGet{*this}};
				}
		};
		////////////////////////////////////////////////////////////////////////////////
	}

	// TODO: One big issue right now is how you have to pass data between functions. Define them
	//       outside everything and then capture by reference. One option might be to pass in
	//       every cache like `.walk(area, cache1, cache2, ...)`. Then have the walker know how
	//       to pull data from each? Then that could be added to the param as a tuple which
	//       would let us do `auto& [cache1Value, cache2Value] = param.values` in the lambda.
	//       That might help a bit, but doesn't solve any other variables and dervied data so
	//       probably not worth.
	/**
	 * Allows you to define function objects ("walkers") to be called at various stages when iterating an area.
	 * 
	 * Iteration order is always X then Y. As such the Y walkers are always called for every
	 * region/chunk/block of that walker type.
	 *
	 * Usage might look something like:
	 *   area = BlockArea/ChunkArea/RegionArea/SpanX
	 *   AreaWalk{area}.onNextRegionY(...).onNextChunkX(...).onNextBlockY(...).walk();
	 *   AreaWalk{area}.onNextRegionY(...).onNextBlockY(...).walk();
	 *   AreaWalk{area}.onNextChunkY(...).walk();
	 */
	class ENGINE_EMPTY_BASE AreaWalk : public Detail::AreaRegionWalkerX<Engine::None, Engine::None> {
		public:
			ENGINE_INLINE constexpr AreaWalk() : AreaRegionWalkerX{Engine::None{}, Engine::None{}} {}

			template<class Next>
			ENGINE_INLINE constexpr AreaRegionWalkerX<Next, Engine::None> onNextRegionX(Next&& next) const noexcept {
				return {std::forward<Next>(next), Engine::None{}};
			}
	};
}

#pragma warning(pop)
#endif
