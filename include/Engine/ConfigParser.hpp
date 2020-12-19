#pragma once

// STD
#include <string>
#include <fstream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine {

	class ConfigParser {
		private:
			using Index = int32;

			struct Token {
				enum class Type : int8 {
					Unknown = 0,
					Whitespace,
					Comment,
					Section,
					Key,
					Assign,
					Value,
					BinLiteral,
					HexLiteral,
					DecLiteral,
				};
				Type type;
				Index start;
				Index stop;
				ENGINE_INLINE Index size() const noexcept { return stop - start + 1; }
				ENGINE_INLINE void reset(Index i) noexcept { start = i; stop = i - 1; }
				std::string_view view(const std::string& data) const { return std::string_view(&data[start], size()); };
			};

		private:
			std::string data;
			Index size;
			Index i;
			Index line;
			Index lineStart;
			const char* err = nullptr;

			std::vector<Token> tokens;

		public:
			void loadAndParse(const std::string& file) {
				tokens.clear();
				tokens.reserve(256);
				i = 0;
				line = 0;
				lineStart = 0;

				data = Utility::readFile(file);
				size = static_cast<Index>(data.size());
				const char* area = nullptr;

				while (!isEOF() && !err) {
					area = nullptr;

					if (eatFill()) { continue; };

					if (data[i] == '[') {
						area = "section definition";
						eatSection();

						eatInlineFill();
						if (!isEOF() && !isNewline()) {
							area = nullptr;
							err = "Unexpected symbols after section definition";
							break;
						}

						continue;
					}

					area = "key definition";
					eatKey();
					if (err) { break; }

					eatFill();

					area = "assignment";
					eatAssign();
					if (err) { break; }

					eatFill();

					area = "value definition";
					eatValue();
					if (err) { break; }
					
					eatInlineFill();
					if (!isEOF() && !isNewline()) {
						area = nullptr;
						err = "Unexpected symbols after value definition";
						break;
					}
				}

				for (const auto& tkn : tokens) {
					std::cout << "Token(" << (int)tkn.type << "): |" << tkn.view(data) << "|\n";
				}

				if (err) {
					ENGINE_WARN("Error parsing config \n",
						file, ":", line + 1, ":", i - lineStart + 1, ": ",
						err, area ? " in " : "", area ? area : "", "\n"
					);
					// TODO: get python like errors:
					//
					// File "<string>", line 3
					//     print("Hello world
                    //                      ^
					//
				} else {
					ENGINE_INFO("Parsed with no errors and ", tokens.size(), " tokens");
				}
			}

		private:
			/**
			 * Checks if the character is a carriage return or line feed.
			 * There are other new line unicode characters such as: VT, FF, CR, NEL, LS, PS
			 * but broad support for those seems iffy so this should be sufficient.
			 * @see https://en.wikipedia.org/wiki/Newline#Unicode
			 */
			ENGINE_INLINE bool isNewline() const noexcept {
				const auto c = data[i];
				return c == '\n' || c == '\r';
			}

			/**
			 * Checks if the character is inline whitespace.
			 * Currently only tabs spaces and newlines are considered whitespace.
			 * @see isNewLine
			 * @see https://en.wikipedia.org/wiki/Whitespace_character
			 */
			ENGINE_INLINE bool isInlineWhitespace() const noexcept {
				const auto c = data[i];
				return c == ' ' || c == '\t';
			}

			ENGINE_INLINE bool isWhitespace() const noexcept {
				return isInlineWhitespace() || isNewline();
			}

			ENGINE_INLINE bool isEOF() const noexcept {
				return i >= size;
			}

			ENGINE_INLINE bool isDigit() const noexcept {
				const auto c = data[i];
				return '0' <= c && c <= '9';
			}

			ENGINE_INLINE bool isBinaryDigit() const noexcept {
				const auto c = data[i];
				return '0' <= c && c <= '1';
			}

			ENGINE_INLINE bool IsUpperCaseLetter() const noexcept {
				const auto c = data[i];
				return 'A' <= c && c <= 'Z';
			}

			ENGINE_INLINE bool isLowerCaseLetter() const noexcept {
				const auto c = data[i];
				return 'a' <= c && c <= 'z';
			}

			ENGINE_INLINE bool isAlphanumeric() const noexcept {
				return isDigit() || isLowerCaseLetter() || IsUpperCaseLetter();
			}

			ENGINE_INLINE bool isHexDigit() const noexcept {
				const auto c = data[i];
				return isDigit()
					|| ('a' <= c && c <= 'f')
					|| ('A' <= c && c <= 'F');
			}

			ENGINE_INLINE bool isSpaceRemaining(Index s) const noexcept {
				return i + s <= size;
			}

			bool eatInlineWhitespace() {
				Token tkn;
				tkn.reset(i);

				while (!isEOF() && isInlineWhitespace()) { ++i; }
				tkn.stop = i - 1;

				if (tkn.size() <= 0) { return false; }
				tkn.type = Token::Type::Whitespace;
				tokens.push_back(tkn);
				return true;
			}

			bool eatWhitespace() {
				Token tkn;
				tkn.reset(i);

				while (!isEOF()) {
					if (isNewline()) {
						++line;
						lineStart = ++i;
					} else if (isInlineWhitespace()) {
						++i;
					} else {
						break;
					}
				}
				tkn.stop = i - 1;

				if (tkn.size() <= 0) { return false; }
				tkn.type = Token::Type::Whitespace;
				tokens.push_back(tkn);
				return true;
			}

			bool eatComment() {
				if (data[i] != '#') { return false; }
				Token tkn;
				tkn.reset(i);
				while (!isEOF() && !isNewline()) { ++i; }
				tkn.stop = i - 1;
				tkn.type = Token::Type::Comment;
				tokens.push_back(tkn);
				return true;
			}

			bool eatFill() {
				// Dont short circuit
				bool found = eatWhitespace() | eatComment();
				while (eatWhitespace() | eatComment()) {}
				return found;
			}

			bool eatInlineFill() {
				// Dont short circuit
				return eatInlineWhitespace() | eatComment();

			}

			bool eatSection() {
				Token tkn;
				tkn.reset(i);
				while (++i) {
					if (isEOF()) { err = "Unexpected end of file"; return false; }
					if (data[i] == ']') { break; }
					if (isNewline()) { err = "Unexpected new line"; return false; }
				}
				tkn.stop = i;

				if (tkn.size() < 2) {
					err = "Incomplete section";
					return false;
				}
				
				tkn.type = Token::Type::Section;
				tokens.push_back(tkn);
				++i;
				return true;
			}

			bool eatKey() {
				Token tkn;
				tkn.reset(i);

				while (!isWhitespace() && !isEOF()) { ++i; }
				tkn.stop = i - 1;
				if (tkn.size() <= 0) {
					err = "No key name given";
					return false;
				}
				
				tkn.type = Token::Type::Key;
				tokens.push_back(tkn);
				return true;
			}

			bool eatAssign() {
				if (data[i] != '=') {
					err = "Expected assignment operator";
					return false;
				}
				Token tkn;
				tkn.reset(i);
				tkn.stop = i;
				tkn.type = Token::Type::Assign;
				++i;
				tokens.push_back(tkn);
				return true;
			}

			bool eatInteger() {
				// TODO: support hex 0x1234
				Token tkn;
				tkn.reset(i);
				// TODO: handle negative
				while (!isEOF() && isDigit()) { ++i; }
				tkn.stop = i;
			}

			bool eatFloat() {
				// TODO: decimal, exponent
				// TODO: impl
				return false;
			}

			bool eatBinaryNumber() {
				if (!isSpaceRemaining(3)) {
					err = "Invalid binary number";
					return false;
				}

				Token tkn;
				tkn.reset(i);

				if (data[i] != '0' || (data[++i] != 'b' && data[i] != 'B')) {
					err = "binary numbers must have format 0b????";
					return false;
				}

				while (++i, !isEOF() && !isWhitespace()) {
					if (!isBinaryDigit()) { break; }
				}
				tkn.stop = i - 1;

				if (tkn.size() < 3) {
					err = "Invalid binary digit";
					return false;
				}

				tkn.type = Token::Type::BinLiteral;
				tokens.push_back(tkn);
				return true;
			}

			bool eatHexNumber() {
				if (!isSpaceRemaining(3)) {
					err = "Invalid hexadecimal number";
					return false;
				}

				Token tkn;
				tkn.reset(i);

				if (data[i] != '0' || (data[++i] != 'x' && data[i] != 'X')) {
					err = "hexadecimal numbers must have format 0x????";
					return false;
				}

				while (++i, !isEOF() && !isWhitespace()) {
					if (!isHexDigit()) { break; }
				}
				tkn.stop = i - 1;

				if (tkn.size() < 3) {
					err = "Invalid binary digit";
					return false;
				}

				tkn.type = Token::Type::HexLiteral;
				tokens.push_back(tkn);
				return true;
			}

			bool eatDecimalNumber() {
			}

			bool eatBool() {
				// TODO: impl
				return false;
			}

			bool eatString() {
				// TODO: impl
				return false;
			}
			
			bool eatValue() {
				const Index pre = i;

				if (eatBinaryNumber()) { return true; }
				i = pre;
				err = nullptr;

				if (eatHexNumber()) { return true; }
				i = pre;
				err = nullptr;

				err = "Unable to parse value";
				return false;
			}
	};
}
