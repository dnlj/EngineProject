#pragma once

#include <mutex>

namespace Game {
	/**
	 * Sink to in-game console.
	 */
	class GameSink {
		private:
			class Line {
				public:
					uintz begin;
					uintz end;
					Engine::Logger::Info info;
			};

			std::mutex mutex;
			std::vector<Line> lines;
			fmt::memory_buffer buffer;
			EngineInstance& engine;

		public:
			GameSink(EngineInstance& engine) : engine{engine} {}
			void write(const Engine::Logger& logger, const Engine::Logger::Info& info, std::string_view format, fmt::format_args args);
			void printToConsole();
	};
}
