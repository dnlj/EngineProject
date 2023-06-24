// FMT
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

// Engine
#include <Engine/engine.hpp>
#include <Engine/Logger.hpp>
#include <Engine/Zip.hpp>
#include <Engine/Clock.hpp>

// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>
#include <Bench/Dist/FixedStrings.hpp>


namespace {
	using Clock = std::chrono::system_clock;
	consteval std::string_view file() noexcept {
		return __FILE__ + sizeof(ENGINE_BASE_PATH);
	}

	consteval std::string_view label() noexcept {
		return "WARN";
	}

	ENGINE_INLINE auto getTime() {
		// Chrono is slightly faster
		if constexpr (true) {
			return std::chrono::time_point_cast<std::chrono::milliseconds>(Clock::now());
		} else {
			return fmt::gmtime(::time(0));
		}
	}
}

BENCH(logging_std_cout) {
	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		std::cout
			<< '[' << Engine::Detail::getDateTimeString().substr(11,8) << ']'
			<< '[' << file() << ']'
			<< '[' << __LINE__ << ']'
			<< '[' << label() << ']'
			<< " This is my test message: "
			<< str << ' '
			<< numF << ' '
			<< numI
			<< '\n';
	}
	ctx.stopSample();
}

BENCH(logging_std_ostringstream) {
	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		std::ostringstream os;
		os
			<< '[' << Engine::Detail::getDateTimeString().substr(11,8) << ']'
			<< '[' << file() << ']'
			<< '[' << __LINE__ << ']'
			<< '[' << label() << ']'
			<< " This is my test message: "
			<< str << ' '
			<< numF << ' '
			<< numI;
		puts(os.str().c_str());
	}
	ctx.stopSample();
}

BENCH(logging_std_ostringstream_color) {
	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		std::ostringstream os;
		os
			<< Engine::ASCII_RED.str
			<< '[' << Engine::Detail::getDateTimeString().substr(11,8) << ']'
			<< '[' << file() << ']'
			<< '[' << __LINE__ << ']'
			<< '[' << label() << ']'
			<< Engine::ASCII_RESET.str
			<< " This is my test message: "
			<< str << ' '
			<< numF << ' '
			<< Engine::ASCII_GREEN.str
			<< numI
			<< Engine::ASCII_RESET.str;
		puts(os.str().c_str());
	}
	ctx.stopSample();
}

BENCH(logging_fmt_single) {
	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		fmt::print(
			"[{:%H:%M:%S%z}][{}][{}][{}] This is my test message: {} {} {}\n",
			getTime(),
			file(),
			__LINE__,
			"WARN",
			str,
			numF,
			numI
		);
	}
	ctx.stopSample();
}

BENCH(logging_fmt_single_color) {
	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		fmt::print(
			fmt::fg(fmt::terminal_color::red),
			"[{:%H:%M:%S%z}][{}][{}][{}]{} This is my test message: {} {} {}\n",
			getTime(),
			file(),
			__LINE__,
			"WARN",
			"\x1b[0m",
			str,
			numF,
			fmt::styled(numI, fmt::fg(fmt::terminal_color::green))
		);
	}
	ctx.stopSample();
}

BENCH(logging_fmt_multi_color) {
	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		fmt::print(
			fmt::fg(fmt::terminal_color::red),
			"[{:%H:%M:%S%z}][{}][{}][{}]",
			getTime(),
			file(),
			__LINE__,
			"WARN"
		);

		fmt::print(
			" This is my test message: {} {} {}\n",
			str,
			numF,
			fmt::styled(numI, fmt::fg(fmt::terminal_color::green))
		);
	}
	ctx.stopSample();
}

BENCH(logging_engine_macro) {
	auto& cfg = Engine::getGlobalConfig<true>();
	cfg.logColor = true;
	cfg.logTimeOnly = true;

	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		Engine::Detail::log<true>(
			"[WARN]",
			Engine::ASCII_RED,
			file(),
			__LINE__,
			"This is my test message: ",
			str,
			' ',
			numF,
			' ',
			Engine::ASCII_GREEN,
			numI,
			Engine::ASCII_RESET
		);
	}
	ctx.stopSample();
}

BENCH(logging_engine_logger) {
	using Engine::Log::Style;
	using Engine::Log::Styled;
	Engine::Logger logger;
	const auto ip = Engine::Net::IPv4Address(000, 111, 222, 101, 12345);
		
	logger.styledWritter = [](Engine::Logger& logger, const Engine::Logger::Info& info, std::string_view format, fmt::format_args args){
		fmt::memory_buffer buffer;
		buffer.clear();
		logger.decorate<true, true>(fmt::appender(buffer), info);
		fmt::vformat_to(fmt::appender(buffer), format, args);
		//std::cout << std::string_view(buffer);
		fwrite(buffer.data(), 1, buffer.size(), stdout);
	};

	ctx.startSample();
	for (const auto& [str, numF, numI] : dataset) {
		logger.warn("This is my test message: {} {} {}\n", str, numF, Styled{numI, Style::Bold | Style::Foreground{2}});
	}
	ctx.stopSample();
}

namespace {
	constexpr auto count = 1000;
	namespace D = Bench::Dist;
	using DataSet1 = Engine::Zip<
		D::FixedStrings<count, 16, 0>,
		D::Uniform<float, count, 0>,
		D::Uniform<int32_t, count, 0>
	>;
}

BENCH_GROUP("logging", 2, 20);

BENCH_USE(logging_std_cout, DataSet1);
BENCH_USE(logging_std_ostringstream, DataSet1);
BENCH_USE(logging_std_ostringstream_color, DataSet1);
BENCH_USE(logging_fmt_single, DataSet1);
BENCH_USE(logging_fmt_single_color, DataSet1);
BENCH_USE(logging_fmt_multi_color, DataSet1);
BENCH_USE(logging_engine_macro, DataSet1);
BENCH_USE(logging_engine_logger, DataSet1);
