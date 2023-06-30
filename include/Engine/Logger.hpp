#pragma once

// STD
#include <source_location>

// FMT
#include <fmt/core.h>

// Engine
#include <Engine/engine.hpp>
#include <Engine/traits.hpp>


namespace Engine::Log {
	enum class Level {
		Default = 0,
		Debug,
		Log,
		Info,
		Success,
		Verbose,
		Warn,
		Error,
		User, // Start of user defined levels
	};
	ENGINE_BUILD_DECAY_ENUM(Level);

	class FormatString {
		public:
			template<class S>
			consteval FormatString(const S& format, std::source_location location = std::source_location::current())
				: format{format}
				, location{location} {
			}
			std::string_view format;
			std::source_location location;
	};

	class StyleBitset {
		public:
			uint32 bitset = {};
			consteval StyleBitset() {}
			consteval StyleBitset(uint32 bitset) : bitset{bitset} {}
			friend consteval StyleBitset operator&(StyleBitset left, StyleBitset right) { return left.bitset & right.bitset; }
			friend consteval StyleBitset operator|(StyleBitset left, StyleBitset right) { return left.bitset | right.bitset; }
			consteval operator bool() const noexcept { return bitset; }
	};

	namespace Detail {
		class NamedColor {
			public:
				enum class UseCase : uint8 {
					None = 0,
					Named, // Original colors + bright variants
					Expanded, // 256 color
					RGB, // True color rgb

				};

				glm::u8vec3 color = {};
				UseCase useCase = {};

			public:
				constexpr NamedColor(None) noexcept {} // TODO (MSVC): ICE if we try to use a default/zero-arg constructor.
				consteval NamedColor(uint8 n) : color{n,0,0}, useCase{UseCase::Expanded} {}
				consteval NamedColor(uint8 n, bool bright) : color{n,0,0}, useCase{UseCase::Named} {}
				consteval NamedColor(glm::u8vec3 color) : color{color}, useCase{UseCase::RGB} {}
				consteval NamedColor(uint8 r, uint8 g, uint8 b) : color{r,g,b}, useCase{UseCase::RGB} {}
				constexpr operator bool() const noexcept { return useCase != UseCase::None; }
		}; static_assert(sizeof(NamedColor) == 4);
	}
	
	class Style {
		public:
			struct Foreground : Detail::NamedColor { using Detail::NamedColor::NamedColor; };
			struct Background : Detail::NamedColor { using Detail::NamedColor::NamedColor; };

			struct FG {
				constexpr static Foreground Black = {30, false};
				constexpr static Foreground Red = {31, false};
				constexpr static Foreground Green = {32, false};
				constexpr static Foreground Yellow = {33, false};
				constexpr static Foreground Blue = {34, false};
				constexpr static Foreground Magenta = {35, false};
				constexpr static Foreground Cyan = {36, false};
				constexpr static Foreground White = {37, false};
				constexpr static Foreground BrightBlack = {90, false};
				constexpr static Foreground BrightRed = {91, false};
				constexpr static Foreground BrightGreen = {92, false};
				constexpr static Foreground BrightYellow = {93, false};
				constexpr static Foreground BrightBlue = {94, false};
				constexpr static Foreground BrightMagenta = {95, false};
				constexpr static Foreground BrightCyan = {96, false};
				constexpr static Foreground BrightWhite = {97, false};
			};

			struct BG {
				constexpr static Background Black = {40, false};
				constexpr static Background Red = {41, false};
				constexpr static Background Green = {42, false};
				constexpr static Background Yellow = {43, false};
				constexpr static Background Blue = {44, false};
				constexpr static Background Magenta = {45, false};
				constexpr static Background Cyan = {46, false};
				constexpr static Background White = {47, false};
				constexpr static Background BrightBlack = {100, false};
				constexpr static Background BrightRed = {101, false};
				constexpr static Background BrightGreen = {102, false};
				constexpr static Background BrightYellow = {103, false};
				constexpr static Background BrightBlue = {104, false};
				constexpr static Background BrightMagenta = {105, false};
				constexpr static Background BrightCyan = {106, false};
				constexpr static Background BrightWhite = {107, false};
			};

			constexpr static StyleBitset Empty = 0;
			constexpr static StyleBitset Bold = 1 << 0;
			constexpr static StyleBitset Faint = 1 << 1;
			constexpr static StyleBitset Italic = 1 << 2;
			constexpr static StyleBitset Underline = 1 << 3;
			constexpr static StyleBitset Invert = 1 << 4;
			constexpr static StyleBitset Reset = 1 << 5;

		private:
			friend class ANSIEscapeSequence;
			StyleBitset bitset = {};
			Foreground fg = None{};
			Background bg = None{};

		public:
			consteval Style(StyleBitset style)
				: bitset{style} {
			}
			consteval Style(StyleBitset style, Foreground fg)
				: bitset{style}, fg{fg} {
			}
			consteval Style(StyleBitset style, Background bg)
				: bitset{style}, bg{bg} {
			}
			consteval Style(StyleBitset style, Foreground fg, Background bg)
				: bitset{style}, fg{fg}, bg{bg} {
			}
			consteval Style(Foreground fg)
				: fg{fg} {
			}
			consteval Style(Background bg)
				: bg{bg} {
			}

			consteval operator bool() const {
				return bitset != 0 || fg || bg;
			}

			friend consteval Style operator|(const Style& left, const Style& right) {
				auto result = left;
				result.bitset = left.bitset | right.bitset;

				if (left.fg && right.fg) {
					throw "Invalid style combination. Both styles have a foreground.";
				} else if (right.fg) {
					result.fg = right.fg;
				}

				if (left.bg && right.bg) {
					throw "Invalid style combination. Both styles have a background.";
				} else if (right.bg) {
					result.bg = right.bg;
				}

				return result;
			}

			friend consteval Style operator|(const Style& left, const Foreground& right) {
				auto result = left;
				if (result.fg) { throw "Style already has a foreground."; }
				result.fg = right;
				return result;
			}

			friend consteval Style operator|(const StyleBitset& left, const Foreground& right) {
				return Style{left} | right;
			}

			friend consteval Style operator|(const Style& left, const Background& right) {
				auto result = left;
				if (result.bg) { throw "Style already has a foreground."; }
				result.bg = right;
				return result;
			}

			friend consteval Style operator|(const StyleBitset& left, const Background& right) {
				return Style{left} | right;
			}
	};

	class ANSIEscapeSequence {
		public:
			constexpr static std::string_view reset = "\033[0m";

		private:
			using Storage = std::array<char, 31>;
			const Storage str = {};
			const uint8 len = 0;

		public:
			consteval auto size() const noexcept {
				using L = decltype(len);
				const auto l = std::find(str.begin(), str.end(), '\0') - str.begin();
				if (std::cmp_greater(l, std::numeric_limits<L>::max())) {
					throw "Style is to large for ANSIEscapeSequence";
				}
				return static_cast<L>(l);
			}

			constexpr explicit ANSIEscapeSequence(None) {};

			consteval ANSIEscapeSequence(Style style)
				: str{from(style)}
				, len{size()} {
			}

			consteval ANSIEscapeSequence(StyleBitset style)
				: str{from(Style{style})}
				, len{size()} {
			}

			consteval ANSIEscapeSequence(Style::Foreground style)
				: str{from(Style{style})}
				, len{size()} {
			}

			consteval ANSIEscapeSequence(Style::Background style)
				: str{from(Style{style})}
				, len{size()} {
			}

			constexpr std::string_view view() const noexcept { return std::string_view{str.data(), len}; }

		private:
			consteval static Storage from(const Style style) {
				// References:
				// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
				// https://en.wikipedia.org/wiki/ANSI_escape_code?useskin=vector#SGR_(Select_Graphic_Rendition)_parameters
				// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

				if (style.bitset & Style::Reset) { return Storage{"\033[0m"}; }
				if (!style) { return Storage{}; }

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

				if (style.bitset & Style::Bold) { append("1"); }
				if (style.bitset & Style::Faint) { append("2"); }
				if (style.bitset & Style::Italic) { append("3"); }
				if (style.bitset & Style::Underline) { append("4"); }
				if (style.bitset & Style::Invert) { append("7"); }

				if (style.fg.useCase == Style::Foreground::UseCase::Named) {
					append(style.fg.color[0]);
				} else if (style.fg.useCase == Style::Foreground::UseCase::Expanded) {
					append("38;5");
					append(style.fg.color[0]);
				} else if (style.fg.useCase == Style::Foreground::UseCase::RGB) {
					append("38;2");
					append(style.fg.color[0]);
					append(style.fg.color[1]);
					append(style.fg.color[2]);
				}

				if (style.bg.useCase == Style::Background::UseCase::Named) {
					append(style.bg.color[0]);
				} else if (style.bg.useCase == Style::Background::UseCase::Expanded) {
					append("48;5");
					append(style.bg.color[0]);
				} else if (style.bg.useCase == Style::Background::UseCase::RGB) {
					append("48;2");
					append(style.bg.color[0]);
					append(style.bg.color[1]);
					append(style.bg.color[2]);
				}

				seq += "m"; // Don't use `append`. We don't want the extra semicolon.
				if (seq.size() > std::tuple_size_v<decltype(str)> - 1) { // -1 for null
					throw "Style string exceeds the char buffer. Increase buffer size.";
				}

				Storage res = {};
				std::ranges::copy(seq.begin(), seq.end(), res.begin());
				return res;
			}
	}; static_assert(sizeof(ANSIEscapeSequence) == 32);

	template<class T>
	class Styled {
		public:
			constexpr Styled(const T& value, ANSIEscapeSequence style)
				: value{value}
				, style{style} {
			}
			const T& value;
			const ANSIEscapeSequence style;
	};

	template<class Char>
	Styled(const std::basic_string<Char>&) -> Styled<std::basic_string_view<Char>>;

	template<AnyChar Char>
	Styled(const Char*&) -> Styled<std::basic_string_view<Char>>;

	template<AnyChar Char, size_t N>
	Styled(const Char(&)[N]) -> Styled<std::basic_string_view<Char>>;

	class Logger {
		private:
			struct StyleFilter {
				template<class T>
				constexpr const T& operator()(const T& value) const noexcept {
					return value;
				}

				template<class T>
				const T& operator()(const Styled<T>& styled) const noexcept {
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
					const Level level;
					const std::string_view label;
					const TimePoint time;
					const ANSIEscapeSequence style;

					constexpr std::string_view relative() const noexcept {
						return location.file_name() + sizeof(ENGINE_BASE_PATH);
					}
			};
			
			using OutputFunc = void (*)(const Engine::Log::Logger& logger, const Engine::Log::Logger::Info& info, std::string_view format, fmt::format_args args);

		public:
			OutputFunc styledWritter = nullptr;
			OutputFunc cleanWritter = nullptr;
			void* userdata = nullptr;

			template<class... Args>
			void debug(FormatString format, const Args&... args) const {
				write(format.location, Level::Debug, "DEBUG", Style::Foreground{3}, format.format, args...);
			}
			
			template<class... Args>
			void log(FormatString format, const Args&... args) const {
				write(format.location, Level::Log, "LOG", Style::FG::BrightBlack, format.format, args...);
			}
			
			template<class... Args>
			void info(FormatString format, const Args&... args) const {
				write(format.location, Level::Info, "INFO", Style::Foreground{4}, format.format, args...);
			}
			
			template<class... Args>
			void success(FormatString format, const Args&... args) const {
				write(format.location, Level::Success, "SUCCESS", Style::Foreground{2}, format.format, args...);
			}
			
			template<class... Args>
			void verbose(FormatString format, const Args&... args) const {
				write(format.location, Level::Verbose, "VERBOSE", Style::Foreground{15}, format.format, args...);
			}
			
			template<class... Args>
			void warn(FormatString format, const Args&... args) const {
				write(format.location, Level::Warn, "WARN", Style::Foreground{3}, format.format, args...);
			}

			template<class... Args>
			void error(FormatString format, const Args&... args) const {
				write(format.location, Level::Error, "ERROR", Style::Foreground{1}, format.format, args...);
			}

			template<class... Args>
			void user(FormatString format, std::underlying_type_t<Level> level, std::string_view label, ANSIEscapeSequence style, const Args&... args) const {
				if constexpr (ENGINE_DEBUG) {
					if (level < +Level::User) { ENGINE_DEBUG_BREAK; }
				}
				write(format.location, static_cast<Level>(level), label, style, format.format, args...);
			}

			template<bool TimeOnly, bool Style, class OutputIt>
			static void decorate(OutputIt out, const Info& info) {
				constexpr auto shortFormat = "[{:%H:%M:%S%z}][{}:{}][{}] ";
				constexpr auto longFormat = "[{:%Y-%m-%dT%H:%M:%S%z}][{}:{}][{}] ";

				if constexpr (Style) {
					std::ranges::copy(info.style.view(), out);
				}

				fmt::format_to(out, TimeOnly ? shortFormat : longFormat, info.time, info.relative(), info.location.line(), info.label);

				if constexpr (Style) {
					constexpr auto reset = ANSIEscapeSequence{Style::Reset};
					std::ranges::copy(reset.view(), out);
				}
			}

		private:
			template<class... Args>
			ENGINE_INLINE void write(const std::source_location location, Level level, std::string_view label, ANSIEscapeSequence style, std::string_view format, const Args&... args) const {
				Info info = {
					.location = location,
					.level = level,
					.label = label,
					.time = std::chrono::time_point_cast<Duration>(Clock::now()),
					.style = style,
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

namespace Engine {
	using Logger = Log::Logger;
}

template<class T, class Char>
struct fmt::formatter<Engine::Log::Styled<T>, Char> : fmt::formatter<T, Char> {
	format_context::iterator format(const Engine::Log::Styled<T>& styled, format_context& ctx) {
		std::ranges::copy(styled.style.view(), ctx.out());
		fmt::format_to(ctx.out(), "{}", styled.value);
		std::ranges::copy("\033[0m", ctx.out());
		return ctx.out();
	}
};
