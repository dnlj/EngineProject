#pragma once

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	class Base16 {
		public:
			//constexpr static char lookup[64] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'};
			constexpr static char toLookup[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
			constexpr static char fromLookup[] = {
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
				0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			};

		private:
			template<class = void> // Template is a work around: https://developercommunity.visualstudio.com/t/inside-a-class-static-asserts-dont-work-for-static/1265685
			constexpr static bool _assert_verify_lookup_table() noexcept {
				bool res = true;
				for (int i = 0; i < std::size(toLookup); ++i) {
					res = res && (fromLookup[toLookup[i]] == i);
				}
				return res;
			}
			static_assert(_assert_verify_lookup_table());

		public:
			static std::string encode(const void* dat, size_t sz) {
				const byte* data = reinterpret_cast<const byte*>(dat);
				std::string res;
				res.reserve(sz * 2 + 1);

				for (int i = 0; i < sz; ++i) {
					res.push_back(toLookup[(data[i] & 0b11110000) >> 4]);
					res.push_back(toLookup[data[i] & 0b00001111]);
				}

				return res;
			}

			static void decode(std::string_view str, void* dat) {
				byte* res = reinterpret_cast<byte*>(dat);
				const auto sz = str.size();
				for (int i = 0; i < sz; ++i) {
					const int a = fromLookup[str[i]] << 4;
					const int b = fromLookup[str[++i]];
					*res++ = a | b;
				}
			}

			static std::vector<byte> decode(std::string_view str) {
				std::vector<byte> res;
				res.resize(str.size() / 2);
				decode(str, res.data());
				return res;
			}
	};
};
