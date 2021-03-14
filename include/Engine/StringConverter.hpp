#pragma once

// STD
#include <string>
#include <string_view>
#include <concepts>
#include <charconv>
#include <limits>

// Engine
#include <Engine/Types.hpp>

namespace Engine {
	enum StringFormatOptions : uint64 {
		Default = 0,
		HexInteger = 1 << 0,
		BinInteger = 1 << 1,
		DecInteger = 1 << 2,
		QuoteEscapeASCII = 1 << 3,
	};

	template<class T>
	class StringConverter;

	template<std::floating_point T>
	class StringConverter<T> {
		public:
			ENGINE_INLINE bool operator()(std::string_view str, T& val) noexcept {
				const char* start = &*str.cbegin();
				const char* const stop = start + str.size();
				if (*start == '+') { ++start; }
				return std::from_chars(start, stop, val).ec == std::errc{};
			}
			
			ENGINE_INLINE bool operator()(const T& val, std::string& str, StringFormatOptions opts) noexcept {
				str.resize(16); // TODO: what size to use?
				const auto res = std::to_chars(&*str.begin(), &*str.begin() + str.size(), val);
				str.resize(res.ptr - &*str.begin());
				return res.ec == std::errc{};
			}
	};

	template<std::integral T>
	class StringConverter<T> {
		public:
			bool operator()(std::string_view str, T& val) noexcept {
				int base = 10;
				const char* start = &*str.cbegin();
				const char* const stop = start + str.size();
				bool neg = false;

				if (*start == '+') { ++start; }
				else if (*start == '-') { neg = true; ++start; }

				while (stop - start > 2) {
					if (*start != '0') { break; }
					++start;
					if (*start == 'x' || *start == 'X') { ++start; base = 16; break; }
					if (*start == 'b' || *start == 'B') { ++start; base = 2; break; }
					base = 8; break;
				}

				const auto res = std::from_chars(start, stop, val, base);
				if (neg) { val = -val; }
				return res.ec == std::errc{};
			}

			bool operator()(T val, std::string& str, StringFormatOptions opts) noexcept {
				int base = 10;

				if (sizeof(T) * 8 <= 32 && (opts & StringFormatOptions::BinInteger)) {
					str.resize(16);
				} else {
					static_assert(sizeof(T) * 8 <= 64); // Types larger than 64 bits will require more space
					str.resize(64 + 3); // +3 for `-0x' prefix
				}

				char* start = &*str.begin();

				if (opts & StringFormatOptions::HexInteger) {
					base = 16;
					*start = '0'; ++start;
					*start = 'x'; ++start;
				} else if (opts & StringFormatOptions::BinInteger) {
					base = 2;
					*start = '0'; ++start;
					*start = 'b'; ++start;
				}

				const auto res = std::to_chars(start, &*str.begin() + str.size(), val, base);

				if (res.ec == std::errc{}) {
					// Fix negative in hex and binary numbers
					if (str[2] == '-') {
						str[2] = str[1];
						str[1] = str[0];
						str[0] = '-';
					}

					str.resize(res.ptr - &*str.begin());
					return true;
				}
				return false;
			}
	};

	template<>
	class StringConverter<bool> {
		public:
			ENGINE_INLINE bool operator()(std::string_view str, bool& val) noexcept {
				val = (str.size() > 0) && (str[0] != '0') && (str[0] != 'f') && (str[0] != 'F');
				return true;
			}
			
			ENGINE_INLINE bool operator()(const bool& val, std::string& str, StringFormatOptions opts) noexcept {
				str = val ? "true" : "false";
				return true;
			}
	};
	
	template<>
	class StringConverter<std::string> {
		public:
			ENGINE_INLINE bool operator()(std::string_view str, std::string& val) noexcept {
				val.assign(str.cbegin(), str.cend());
				return true;
			}
			
			ENGINE_INLINE bool operator()(const std::string& val, std::string& str, StringFormatOptions opts) noexcept {
				if (opts & StringFormatOptions::QuoteEscapeASCII) {
					str.resize(val.size() * 2 + 2);
					auto out = str.begin();
					*out++ = '"';
					auto curr = val.cbegin();
					auto last = curr;
					const auto stop = val.cend();

					while (curr != stop) {
						switch (*curr) {
							case '\\': { out = std::copy(last, curr, out); *out++ = '\\'; last = curr; break; }
							case '"': { out = std::copy(last, curr, out); *out++ = '\\'; last = curr; break; }
						}
						++curr;
					}

					out = std::copy(last, curr, out);
					*out++ = '"';
					str.resize(out - str.begin());
				} else {
					str = val;
				}
				return true;
			}
	};
}
