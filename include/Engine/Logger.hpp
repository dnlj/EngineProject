#pragma once

// STD
#include <source_location>

// FMT
#include <fmt/core.h>

// Engine
#include <Engine/traits.hpp>


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


	class ANSIStyle {
		public:
			enum Style {
				Empty = 0,
				Bold = 1 << 0,
				Faint = 1 << 1,
				Italic = 1 << 2,
				Underline = 1 << 3,
			};

			class NamedColor {
				public:
					const glm::u8vec3 color = {};
					const uint8 useCase = 0; // 0 = none, 1 = bit, 2 = rgb

				public:
					consteval NamedColor(nullptr_t) {}; // TODO (MSVC): ICE if we try to use a default constructor.
					consteval NamedColor(uint8 n) : color{n,0,0}, useCase{1} {}
					consteval NamedColor(glm::u8vec3 color) : color{color}, useCase{2} {}
			}; static_assert(sizeof(NamedColor) == 4);

			struct Foreground : NamedColor { using NamedColor::NamedColor; };
			struct Background : NamedColor { using NamedColor::NamedColor; };

		public:
			consteval ANSIStyle(Style style)
				: bitset{style} {
			}
			consteval ANSIStyle(Style style, Foreground fg)
				: bitset{style}, fg{fg} {
			}
			consteval ANSIStyle(Style style, Background bg)
				: bitset{style}, bg{bg} {
			}
			consteval ANSIStyle(Style style, Foreground fg, Background bg)
				: bitset{style}, fg{fg}, bg{bg} {
			}

			Style bitset;
			Foreground fg = nullptr_t{};
			Background bg = nullptr_t{};

			// TODO: combine styles with | instead of constructor{,,,,}
			//friend ANSIStyle operator|(const ANSIStyle& left, const ANSIStyle& right) {}
			//friend ANSIStyle operator|(const Style& left, const ANSIStyle& right) {}
			//friend ANSIStyle operator|(const ANSIStyle& left, const Style& right) {}
			//friend ANSIStyle operator|(const ANSIStyle& left, const Style& right) {}
	};
	ENGINE_BUILD_ALL_OPS(ANSIStyle::Style);
	ENGINE_BUILD_DECAY_ENUM(ANSIStyle::Style);


	consteval auto styleFor(ANSIStyle style) {
	}

	class ANSIEscapeSequence {
		//
		// Bold = 1
		// Faint = 2
		// Italic = 3
		// Underline = 4
		//
		// Foreground = 38;5;(n)m
		// Foreground(RGB) = 38;2;(r);(g);(b)m
		//
		// Background = 48;5;(n)m
		// Background(RGB) = 48;2;(r);(g);(b)m
		//
		private:
			const std::array<char, 31> str = {};
			const uint8 len = 0;

		public:
			consteval ANSIEscapeSequence(ANSIStyle style)
				: str{from(style)}
				, len{static_cast<decltype(len)>(std::find(str.begin(), str.end(), '\0') - str.begin())} {
			}
			consteval ANSIEscapeSequence(ANSIStyle::Style style)
				: str{from(ANSIStyle{style})}
				, len{static_cast<decltype(len)>(std::find(str.begin(), str.end(), '\0') - str.begin())} {
			}

			constexpr std::string_view view() const noexcept { return std::string_view{str.data(), len}; }

		public: // TODO: private
			consteval static decltype(str) from(const ANSIStyle style) {
				std::string seq = "\033[";

				const auto append = [&]<class T>(const T& val){
					if (seq.back() != '[') {
						seq += ';';
					}

					if constexpr (std::integral<T>) {
						constexpr auto len = 3;
						seq.append(len, 0);
						auto end = std::to_address(seq.end());
						const auto res = std::to_chars(end - len, end, val);
						
						if (res.ec != std::errc{}) {
							throw "Number is to large";
						}
						
						seq.resize(res.ptr - seq.data());
					} else {
						seq += val;
					}
				};

				if (+(style.bitset & ANSIStyle::Style::Bold)) { append("1"); }
				if (+(style.bitset & ANSIStyle::Style::Faint)) { append("2"); }
				if (+(style.bitset & ANSIStyle::Style::Italic)) { append("3"); }
				if (+(style.bitset & ANSIStyle::Style::Underline)) { append("4"); }

				// TODO: Need to handle the BRIGHT variant in the non-rgb version: https://ss64.com/nt/syntax-ansi.html
		
				// TODO: Verify not normal and RGB set
				if (style.fg.useCase == 1) {
					append("38;5");
					append(style.fg.color[0]);
				} else if (style.fg.useCase == 2) {
					// TODO: rgb
				}

				// TODO: background

				seq += "m"; // Don't use `append`. We don't want the extra semicolon.
				if (seq.size() > std::tuple_size_v<decltype(str)> - 1) { // -1 for null
					throw "Style string exceeds the char buffer. Increase buffer size.";
				}

				// TODO: verify that we have SOME style, shouldn't be empty
				// 
				// TODO: can we use std inserter and just cut out the std::string?
				std::remove_cvref_t<decltype(str)> res = {};
				std::copy(seq.begin(), seq.end(), res.begin());
				return res;
			}
	}; static_assert(sizeof(ANSIEscapeSequence) == 32);

	// TODO: better interface
	template<class T>
	class LogStyle {
		public:
			constexpr LogStyle(const T& value, ANSIEscapeSequence style)
				: value{value}
				, style{style} {
			}
			const T& value;
			const ANSIEscapeSequence style;
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
		std::ranges::copy(styled.style.view(), ctx.out());
		fmt::format_to(ctx.out(), "{}", styled.value);
		std::ranges::copy("\033[0m", ctx.out());
		return ctx.out();
	}
};
