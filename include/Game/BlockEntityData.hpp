#pragma once

// STD
#include <type_traits>

// Game
#include <Game/BlockEntityType.hpp>


namespace Game {
	template<BlockEntityType Type>
	class BlockEntityTypeData {
		static_assert(Type != Type, "Attempting to get BlockEntityData for non-specialized type.");
	};
	
	template<>
	class BlockEntityTypeData<BlockEntityType::None> {
	};

	template<>
	class BlockEntityTypeData<BlockEntityType::Tree> {
		public:
			uint8 type;
			glm::vec2 size; // TODO: this should probably be infered from type.
	};

	template<>
	class BlockEntityTypeData<BlockEntityType::Portal> {
		public:
			RealmId realmId;
			BlockVec blockPos;
	};

	class BlockEntityDataUnion {
		public:
			union {
				#define X(Name) BlockEntityTypeData<BlockEntityType::Name> as##Name;
				#include <Game/BlockEntityType.xpp>
			};
	};

	class BlockEntityData : public BlockEntityDataUnion {
		public:
			// We cannot simple replace this with an is_trivial because trivial types do not need trivial destructors.
			// So it is better to be explicit with what we want.
			static_assert(std::is_trivially_constructible_v<BlockEntityDataUnion>, "If BlockEntityDataUnion is not trivial we must implement rule of N.");
			static_assert(std::is_trivially_copyable_v<BlockEntityDataUnion>, "If BlockEntityDataUnion is not trivial we must implement rule of N.");
			static_assert(std::is_trivially_destructible_v<BlockEntityDataUnion>, "If BlockEntityDataUnion is not trivial we must implement rule of N.");

			BlockEntityType type;

			template<BlockEntityType T>
			ENGINE_INLINE BlockEntityTypeData<T>& as() noexcept { static_assert(T != T, "Missing specialization for type."); }

			template<BlockEntityType T>
			ENGINE_INLINE const BlockEntityTypeData<T>& as() const noexcept { return const_cast<BlockEntityData*>(this)->as<T>(); }

			#define X(Name) template<> ENGINE_INLINE BlockEntityTypeData<BlockEntityType::Name>& as<BlockEntityType::Name>() noexcept { return as##Name; }
			#include <Game/BlockEntityType.xpp>

			// if we need non trivial types we will have to implement move/copy/assign/~/etc. and switch (with) based ont type.
			/*
			BlockEntityData() {}

			BlockEntityData(const BlockEntityData& other) {
				//with([&]<BlockEntityType T>(auto& data){
				//	data = other.as<T>();
				//});
			}

			BlockEntityData(BlockEntityData&& other) {}
			BlockEntityData& operator=(const BlockEntityData& other) {}
			BlockEntityData& operator=(const BlockEntityData&& other) {}

			~BlockEntityData() {
				with([]<BlockEntityType T, class D>(D& obj){
					obj.~D();
				});
			}
			*/

			template<class Func>
			ENGINE_INLINE void with(Func&& func) {
				switch(type) {
					#define X(Name) case BlockEntityType::Name: { func.operator()<BlockEntityType::Name>(as##Name); break; }
					#include<Game/BlockEntityType.xpp>
					default: {
						ENGINE_WARN("Unknown BlockEntityType. Ignoring.");
					}
				}
			}

			template<class Func>
			ENGINE_INLINE void with(Func&& func) const {
				switch(type) {
					#define X(Name) case BlockEntityType::Name: { func.operator()<BlockEntityType::Name>(as##Name); break; }
					#include<Game/BlockEntityType.xpp>
					default: {
						ENGINE_WARN("Unknown BlockEntityType. Ignoring.");
					}
				}
			}
	};

	class BlockEntityDesc {
		public:
			glm::ivec2 pos;
			BlockEntityData data;
	};
}
