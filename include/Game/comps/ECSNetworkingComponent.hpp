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
				None = 0 << 0,

				Current     = 1 << 0,
				AddedCurr   = 1 << 1,
				AddedPrev   = 1 << 2,
				RemovedCurr = 1 << 3,
				RemovedPrev = 1 << 4,

				ZoneChanged = 1 << 5,
				
				Added       = AddedCurr | AddedPrev,
				Removed     = RemovedCurr | RemovedPrev,
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

					ENGINE_INLINE constexpr void update(NeighborState change) noexcept {
						if ((change & NeighborState::ChangedStates) != NeighborState::None) {
							state &= ~NeighborState::ChangedStates;
						}

						state |= change;
					}
			};

			// TODO: hashmap would probably be better suited here.
			Engine::SparseSet<Engine::ECS::Entity, NeighborData> neighbors;

			// We need to track when the players zone has changed in addition to
			// the neighbors so that ECS_ZONE_INFO still gets sent even if there
			// are no entities near by.
			bool plyZoneChanged = false;
	};
}
