// Game
//#include <Game/common.hpp>
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class RealmManagementSystem final : public System, public PhysicsListener {
		private:
			class RealmChange {
				public:
					Engine::ECS::Entity ent;
					RealmId realmId;
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
