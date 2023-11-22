#pragma once


// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Net/Replication.hpp>

// Game
#include <Game/Connection.hpp>


namespace Game {
	template<class T>
	class NetworkTraits {
		private:
			using _t_NetworkTraits_isSpecialized = int;

		public:
			static Engine::Net::Replication getReplType(const T& obj) {
				static_assert(!sizeof(T), "NetworkTraits::getReplType must be implemented for type T.");
			}

			static void writeInit(const T& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				static_assert(!sizeof(T), "NetworkTraits::writeInit must be implemented for type T.");
			}

			static void write(const T& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				static_assert(!sizeof(T), "NetworkTraits::write must be implemented for type T.");
			}
			
			static std::tuple<> readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				static_assert(!sizeof(T), "NetworkTraits::readInit must be implemented for type T.");
				return std::make_tuple();
			}

			static void read(T& obj, Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				static_assert(!sizeof(T), "NetworkTraits::read must be implemented for type T.");
			}
	};

	template<class T>
	concept IsNetworkedComponent = !requires (typename NetworkTraits<T>::_t_NetworkTraits_isSpecialized special) {
		special;
	};

	template<class F>
	constexpr static inline bool IsNetworkedFlag = false;
}
