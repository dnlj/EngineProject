#pragma once

// FMT
#include <fmt/format.h>

// Engine
#include <Engine/GlobalConfig.hpp>
#include <Engine/Constants.hpp>
#include <Engine/Types.hpp>

// STD
#include <sstream>
#include <ostream>


// TODO: where to put this stuff? we should make something include glm/box2d if they dont use it. May need some kind of integrations/glm.hpp or similar. Glue?
namespace Engine {
	template<class T> struct LogFormatter {
		static void format(std::ostream& stream, const T& val) {
			stream << val;
		}
	};

	template<int L, class T, glm::qualifier Q>
	struct LogFormatter<glm::vec<L, T, Q>> {
		static void format(std::ostream& stream, const glm::vec<L, T, Q>& val) {
			stream << "glm::vec(" << val.x << ", " << val.y << ")";
		}
	};

	template<int C, int R, class T, glm::qualifier Q>
	struct LogFormatter<glm::mat<C, R, T, Q>> {
		static void format(std::ostream& stream, const glm::mat<C, R, T, Q>& val) {
			stream << "glm::mat(";
			for (int r = 0; r < C; ++r) {
				stream << "{";
				for (int c = 0; c < C; ++c) {
					stream << val[c][r];
					if (c < C-1) { stream << ", "; }
				}
				stream << "}";
				if (r < R-1) { stream << ", "; }
			}
			stream << ")";

		}
	};
};

// TODO: Where to put overloads for 3rd party types?
template<int L, class T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> : fmt::formatter<T> {
	template<class FormatContext>
	auto format(const glm::vec<L, T, Q>& vec, FormatContext& ctx) -> decltype(ctx.out()) {
		fmt::format_to(ctx.out(), "(");
		for (int i = 0;;) {
			fmt::formatter<T>::format(vec[i], ctx);
			if (++i == L) { break; }
			fmt::format_to(ctx.out(), ", ");
		}
		return fmt::format_to(ctx.out(), ")");
	}
};

// TODO: Where to put overloads for 3rd party types?
struct b2Vec2;
template<>
struct fmt::formatter<b2Vec2> : fmt::formatter<glm::vec2> {
	// TODO: Once moved: remove the `std::same_as<b2Vec2> auto` workaround. Its
	//       currently just there to defer type resolution so we don't need to
	//       pollute everything with b2_math.h since this file is included
	//       everywhere.
	template<class FormatContext>
	auto format(const std::same_as<b2Vec2> auto& vec, FormatContext& ctx) -> decltype(ctx.out()) {
		return fmt::formatter<glm::vec2>::format({vec.x, vec.y}, ctx);
	}
};

namespace Engine::Detail {
	std::string getDateTimeString();

	// TODO: C++20: Use std <format> and fallback to operator<< if no format found.
	template<bool decorate, class... Args>
	void log(
		std::string_view prefix,
		ASCIIColorString color,
		std::string_view file,
		int line,
		Args&&... args) {

		const auto& gc = Engine::getGlobalConfig();

		std::ostringstream stream;
		decltype(auto) filter = [&]<class V>(V&& value) -> decltype(auto) {
			if constexpr (std::is_same_v<std::decay_t<V>, ASCIIColorString>) {
				if (gc.logColor) {
					return value.str;
				} else {
					constexpr const char* const empty = "";
					return empty;
				}
			} else {
				return std::forward<V>(value);
			}
		};

		if constexpr (decorate) {
			const auto date = getDateTimeString();
			stream
				<< filter(color)
				<< "[" << (gc.logTimeOnly ? std::string_view{date}.substr(11,8) : date) << "]"
				<< "[" << file << ":" << line << "]"
				<< prefix
				<< filter(Engine::ASCII_RESET)
				<< " ";
		}

		(LogFormatter<std::decay_t<decltype(filter(std::forward<Args>(args)))>>::format(stream, filter(std::forward<Args>(args))), ...);
		//(stream << ... << filter(std::forward<Args>(args)));
		stream << '\n';

		// TODO: C++20 ostringstream.view() to avoid copy
		fputs(stream.str().c_str(), gc.log.get());

		// TODO: cmd line option for if should also log to file
		// TODO: when logging to file use full date and no color
	}
}
