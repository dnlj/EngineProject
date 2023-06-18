#pragma once

// STD
#include <source_location>

// FMT
#include <fmt/core.h>

// Engine
#include <Engine/traits.hpp>

// TODO:
//   Verbose
//   Info
//   Debug
//   Error
//   None

namespace Engine {
	enum class LogLevel {
		Default = 0,
		Debug,
		Text,
		Info,
		Verbose,
		Warn,
		Error,
		User, // Start of user defined levels
	};

	class LogFormatString {
		public:
			template<class S>
			consteval LogFormatString(const S& format, std::source_location location = std::source_location::current())
				: format{format}
				, location{location} {
			}
			std::string_view format;
			std::source_location location;
	};

	// TODO: better interface
	template<class T>
	class LogStyle {
		public:
			LogStyle(const T& value) : value{value} {}
			const T& value;
	};

	template<class Char>
	LogStyle(const std::basic_string<Char>&) -> LogStyle<std::basic_string_view<Char>>;

	template<AnyChar Char>
	LogStyle(const Char*&) -> LogStyle<std::basic_string_view<Char>>;

	template<AnyChar Char, size_t N>
	LogStyle(const Char(&)[N]) -> LogStyle<std::basic_string_view<Char>>;

	class Logger {
		private:
			struct StyleFilter {
				template<class T>
				constexpr const T& operator()(const T& value) const noexcept {
					return value;
				}

				template<class T>
				const T& operator()(const LogStyle<T>& styled) const noexcept {
					return styled.value;
				};
			};

		public:
			using Clock = std::chrono::system_clock;
			using Duration = std::chrono::milliseconds;
			using TimePoint = std::chrono::time_point<Clock, Duration>; // {fmt} doesn't let you specify ms when formatting time points so we have to force it with a cast.

			class Info {
				public:
					const std::source_location location;
					const LogLevel level;
					const std::string_view label;
					const TimePoint time;

					constexpr std::string_view relative() const noexcept {
						return location.file_name() + sizeof(ENGINE_BASE_PATH);
					}
			};
			
			using LogFunc = void (*)(Engine::Logger& logger, const Engine::Logger::Info& info, std::string_view format, fmt::format_args args);

		public:
			LogFunc styledWritter = nullptr;
			LogFunc cleanWritter = nullptr;
			void* userdata = nullptr;
			
			//template<class... Args>
			//void warn(LogFormatString<Args...> format, const Args&... args);

			template<class... Args>
			void warn(LogFormatString format, const Args&... args) {
				log(format.location, LogLevel::Warn, "WARN", format.format, args...);
			}

			template<bool TimeOnly, bool Style, class OutputIt>
			static void decorate(OutputIt out, const Info& info) {
				constexpr auto shortFormat = "[{:%H:%M:%S%z}][{}][{}][{}] ";
				constexpr auto longFormat = "[{:%Y-%m-%dT%H:%M:%S%z}][{}][{}][{}] ";
				if constexpr (Style) { std::ranges::copy("\033[31m", out); } // TODO: Don't hard code styles
				fmt::format_to(out, TimeOnly ? shortFormat : longFormat, info.time, info.relative(), info.location.line(), info.label);
				if constexpr (Style) { std::ranges::copy("\033[0m", out); } // TODO: Don't hard code styles
			}

		private:
			template<class... Args>
			void log(const std::source_location location, LogLevel level, std::string_view label, std::string_view format, const Args&... args) {
				Info info = {
					.location = location,
					.level = level,
					.label = label,
					.time = std::chrono::time_point_cast<Duration>(Clock::now()),
				};

				if (styledWritter) {
					styledWritter(*this, info, format, fmt::make_format_args(args...));
				}
				if (cleanWritter) {
					cleanWritter(*this, info, format, fmt::make_format_args(StyleFilter{}(args)...));
				}
			}

	};
}


template<class T, class Char>
struct fmt::formatter<Engine::LogStyle<T>, Char> : fmt::formatter<T, Char> {
	format_context::iterator format(const Engine::LogStyle<T>& styled, format_context& ctx) {
		std::ranges::copy("\033[34m", ctx.out());
		fmt::format_to(ctx.out(), "{}", styled.value);
		std::ranges::copy("\033[0m", ctx.out());
		return ctx.out();
	}
};
