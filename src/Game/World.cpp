#include <Game/World.hpp>
#include <Game/systems/all.hpp>
#include <Game/comps/all.hpp>


#define ECS_WORLD_TYPE ::Game::World::BaseType
#define ECS_WORLD_ARG std::tuple<Game::World&, Game::EngineInstance&>&&
#include <Engine/ECS/World.ipp>
