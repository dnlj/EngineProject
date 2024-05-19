// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class RealmManagementSystem final : public System, public PhysicsListener {
		private:
			class RealmChange {
				public:
					Engine::ECS::Entity ply;
					RealmId realmId;
					BlockVec pos;
			};

		public:
			RealmManagementSystem(SystemArg arg);
			void setup();
			void tick();
			void beginContact(const Engine::ECS::Entity entA, const Engine::ECS::Entity entB) override;

		private:
			std::vector<RealmChange> changes;
	};
}
