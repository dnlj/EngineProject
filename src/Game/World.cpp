#include <Game/World.hpp>
#include <Game/systems/all.hpp>


// TODO: these shouldnt be here. figure out how to handle this better
#define WORLD_TPARAMS template<\
	int64 TickRate,\
	class... Ss,\
	template<class...> class SystemsSet,\
	class... Cs,\
	template<class...> class ComponentsSet\
>

#define WORLD_CLASS World<TickRate, SystemsSet<Ss...>, ComponentsSet<Cs...>>

#include <Engine/ECS/World.ipp>

// TODO: these shouldnt be here. figure out how to handle this better
#undef WORLD_TPARAMS
#undef WORLD_CLASS

template Engine::ECS::World<Game::tickrate, Game::SystemsSet, Game::ComponentsSet>::World(std::tuple<Game::World&, Game::EngineInstance&>&&);
template void Engine::ECS::World<Game::tickrate, Game::SystemsSet, Game::ComponentsSet>::run();
template Engine::ECS::World<Game::tickrate, Game::SystemsSet, Game::ComponentsSet>::~World();
