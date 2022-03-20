#include <Game/World.hpp>
#include <Game/systems/all.hpp>
#include <Game/comps/all.hpp>


// TODO: move into ipp file or just dont undef in hpp file
#define WORLD_TPARAMS template<\
	int64 TickRate,\
	class... Ss,\
	template<class...> class SystemsSet,\
	class... Ns_,\
	template<class...> class NonFlagsSet_,\
	class... Fs_,\
	template<class...> class FlagsSet_,\
	class... Cs,\
	template<class...> class ComponentsSet\
>

#define WORLD_CLASS World<TickRate, SystemsSet<Ss...>, NonFlagsSet_<Ns_...>, FlagsSet_<Fs_...>, ComponentsSet<Cs...>>

#include <Engine/ECS/World.ipp>

// TODO: move into ipp file or just dont undef in hpp file
#undef WORLD_TPARAMS
#undef WORLD_CLASS


// TODO: if we use a macro we should be able to just shove this in the ipp file
template Game::World::BaseType::World(std::tuple<Game::World&, Game::EngineInstance&>&&);
template void Game::World::BaseType::run();
template Game::World::BaseType::~World();
