#pragma once

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
	namespace Detail {
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
		class ENGINE_EMPTY_BASE AreaBlockWalkerY : public AreaBlockWalkerYGet<Func, ParentGet> {
			public:
				using BaseGet = AreaBlockWalkerYGet<Func, ParentGet>;

				ENGINE_INLINE constexpr AreaBlockWalkerY(Func&& func, ParentGet&& parent)
					: BaseGet{std::move(func), std::move(parent)} {
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
	
	/**
	 * Allows you to define function objects ("walkers") to be called at various stages when iterating an area.
	 * 
	 * Iteration order is always X then Y. As such the Y walkers are always called for every
	 * region/chunk/block of that walker type.
	 *
	 * Usage might look something like:
	 *   area = BlockArea/ChunkArea/RegionArea/SpanX
	 *   AreaWalker{area}.onNextRegionY(...).onNextChunkX(...).onNextBlockY(...).walk();
	 *   AreaWalker{area}.onNextRegionY(...).onNextBlockY(...).walk();
	 *   AreaWalker{area}.onNextChunkY(...).walk();
	 */
	class ENGINE_EMPTY_BASE AreaWalker : public Detail::AreaRegionWalkerX<Engine::None, Engine::None> {
		public:
			ENGINE_INLINE constexpr AreaWalker() : AreaRegionWalkerX{Engine::None{}, Engine::None{}} {}

			template<class Next>
			ENGINE_INLINE constexpr AreaRegionWalkerX<Next, Engine::None> onNextRegionX(Next&& next) const noexcept {
				return {std::forward<Next>(next), Engine::None{}};
			}
	};
}

#pragma warning(pop)
