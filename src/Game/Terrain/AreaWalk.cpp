// Game
#include <Game/Terrain/AreaWalk.hpp>


// TODO: move to test project if we ever get that back up and running.
void test_AreaWalker() {
		using Game::Terrain::AreaWalk;

		constexpr auto funcWithCapture1 = [v = 0x1234ll]{}; static_assert(sizeof(funcWithCapture1) == sizeof(long long));
		constexpr auto funcWithCapture2 = [v = 0x4321ll]{}; static_assert(sizeof(funcWithCapture2) == sizeof(long long));

		{ // Object sizes are as expected. EBO is working as intended.
			auto a = AreaWalk{}
				.onNextRegionX([]{});
			static_assert(sizeof(a) == 1);

			auto b = AreaWalk{}
				.onNextRegionY([]{})
				.onNextChunkY([]{})
				.onNextBlockY([]{});
			static_assert(sizeof(b) == 1);

			auto c = AreaWalk{}
				.onNextRegionX(funcWithCapture1)
				.onNextRegionY([]{});
			static_assert(sizeof(c) == sizeof(funcWithCapture1));

			auto d = AreaWalk{}
				.onNextRegionX([]{})
				.onNextRegionY(funcWithCapture1);
			static_assert(sizeof(d) == sizeof(funcWithCapture1));

			auto e = AreaWalk{}
				.onNextRegionY([]{})
				.onNextBlockX(funcWithCapture1)
				.onNextBlockY([]{});
			static_assert(sizeof(e) == sizeof(funcWithCapture1));

			auto f = AreaWalk{}
				.onNextRegionY([]{})
				.onNextChunkX(funcWithCapture1)
				.onNextChunkY(funcWithCapture2)
				.onNextBlockY([]{});
			static_assert(sizeof(f) == sizeof(funcWithCapture1) + sizeof(funcWithCapture2));
		}

		{ // Skipping stages works as expected. Can be used as constexpr.

			constexpr auto walk = AreaWalk{}
				.onNextRegionY(decltype(funcWithCapture1){funcWithCapture1})
				.onNextChunkY(decltype(funcWithCapture2){funcWithCapture2})
				.onNextBlockY([]{});
			static_assert(sizeof(walk) == sizeof(funcWithCapture1) + sizeof(funcWithCapture2));

			// Shouldn't compile, we didn't define them for the walker.
			//walk.getBlockWalkerX();
			//walk.getChunkWalkerX();
			//walk.getRegionWalkerX();

			walk.getBlockWalkerY();
			walk.getChunkWalkerY();
			walk.getRegionWalkerY();
		}

		{ // A walk with two functions of the same type return unique instances.
			auto func1 = [v = 0] mutable { return ++v; };
			auto func2 = func1;

			auto walk = AreaWalk{}
				.onNextRegionX([]{})
				.onNextRegionY(std::move(func1))
				.onNextChunkX([]{})
				.onNextChunkY(std::move(func2));
			static_assert(sizeof(walk) == sizeof(func1) + sizeof(func2));

			// Debugging
			//const auto fa = std::addressof(walk.getRegionWalkerY());
			//const auto fb = std::addressof(walk.getChunkWalkerY());

			// Initially they have the same value.
			auto& walkerRegionY = walk.getRegionWalkerY();
			auto& walkerChunkY = walk.getChunkWalkerY();
			assert(walkerRegionY() == walkerChunkY());

			// Incrementing one does not increment the other, indicating that they do not
			// share the same object.
			walkerRegionY();
			assert(walkerRegionY() != walkerChunkY());
		}
}
