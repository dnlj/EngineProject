// Game
#include <Game/GameSink.hpp>
#include <Game/systems/UISystem.hpp>
#include <Game/UI/ConsoleWindow.hpp>

namespace Game {
	void GameSink::write(const Engine::Logger& logger, const Engine::Logger::Info& info, std::string_view format, fmt::format_args args) {
		std::lock_guard lock{mutex};
		auto out = std::back_inserter(buffer);

		// Don't prepend user defined levels
		if (info.level < Engine::Log::Level::User) {
			out = '[';
			buffer.append(info.label);
			out = ']';
			out = ' ';
		}

		Line line{
			.begin = buffer.size(),
			.info = info,
		};

		fmt::vformat_to(out, format, std::move(args));
		line.end = buffer.size();
		lines.push_back(line);
	}

	void GameSink::printToConsole() {
		std::lock_guard lock{mutex};

		// Ensure message are printed chronologically since they may be queued from mutliple threads.
		std::sort(lines.begin(), lines.end(), [](const Line& left, const Line& right){
			return left.info.time < right.info.time;
		});

		auto& uiSys = engine.getWorld().getSystem<UISystem>();
		const auto* const data = buffer.data();
		for (const auto& line : lines) {
			uiSys.getConsole()->push(std::string_view{data + line.begin, data + line.end});
		}

		lines.clear();
		buffer.clear();
	}
}
