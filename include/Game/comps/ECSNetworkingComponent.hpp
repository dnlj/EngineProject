#pragma once

// STD
#include <set>

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/SparseSet.hpp>


namespace Game {
	class ECSNetworkingComponent {
		public:
			enum class NeighborState {
				// Not a neighbor. Wasn't added, isn't current, wasn't removed.
				// It does not exist as far as neighbors are concerned.
				//
				// It is almost certainly an error if you are seeing this
				// outside of EntityNetworkingSystem::network.
				None = 0 << 0, 

				Current     = 1 << 0,
				AddedCurr   = 1 << 1, // Added in the current tick.
				AddedAny    = 1 << 2, // Added at any point since last network.
				RemovedCurr = 1 << 3, // Removed in the current tick.
				RemovedAny  = 1 << 4, // Removed at any point since the last network.

				ZoneChanged = 1 << 5,

				// _Might_ be removed. Used by EntityNetworkingSystem
				// internally. Its a bug if you are seeing this outside of
				// EntityNetworkingSystem::updateNeighbors.
				MaybeRemoved = 1 << 6,
				
				Added       = AddedCurr | AddedAny,
				Removed     = RemovedCurr | RemovedAny,
				ChangedStates = Current | Added | Removed,
			};
			ENGINE_BUILD_ALL_OPS_F(NeighborState, friend);

			class NeighborData {
				private:
					NeighborState state{};

				public:
					Engine::ECS::ComponentBitset comps{};

				public:
					ENGINE_INLINE constexpr NeighborData(NeighborState state) : state{state} {}
					ENGINE_INLINE constexpr NeighborState get() const noexcept { return state; }
					ENGINE_INLINE constexpr bool test(NeighborState state) const noexcept { return static_cast<bool>(this->state & state); }
					ENGINE_INLINE constexpr void reset(NeighborState state) noexcept { this->state = state; }

					ENGINE_INLINE constexpr void zoneChanged() noexcept {
						state |= NeighborState::ZoneChanged;
					}

					ENGINE_INLINE constexpr void maybeRemoved() noexcept {
						state &= ~(NeighborState::Current | NeighborState::AddedCurr | NeighborState::RemovedCurr);
						state |= NeighborState::MaybeRemoved;
					}

					ENGINE_INLINE constexpr void removed() noexcept {
						// If it was added and removed in the same update just clear
						// everything and pretend it doesn't/never existed.
						if (test(NeighborState::Added)) {
							state = NeighborState::None;
						} else {
							// If it was removed it must not be current.
							state &= ~(NeighborState::Current | NeighborState::Added);
							state |= NeighborState::Removed;
						}
					}

					ENGINE_INLINE constexpr void current() noexcept {
						// If it was removed, but is now again current, it must
						// not be removed.
						state &= ~(NeighborState::Removed | NeighborState::MaybeRemoved | NeighborState::AddedCurr);
						state |= NeighborState::Current;
					}
			};

			Engine::SparseSet<Engine::ECS::Entity, NeighborData> neighbors;

			// We need to track when the players zone has changed in addition to
			// the neighbors so that ECS_ZONE_INFO still gets sent even if there
			// are no entities near by.
			bool plyZoneChanged = false;
	};
}
