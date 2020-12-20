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
					BinLiteral,
					HexLiteral,
					DecLiteral,
					FloatLiteral,
					BoolLiteral,
					StringLiteral,
					_COUNT,
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

						#define GENERR(T) case Token::Type::T: { err = "Unexpected symbols after " #T " value definition"; break; }
						switch (tokens.back().type) {
							GENERR(BinLiteral);
							GENERR(HexLiteral);
							GENERR(DecLiteral);
							GENERR(FloatLiteral);
							GENERR(BoolLiteral);
							GENERR(StringLiteral);
							default: { err = "Unexpected symbols after value definition"; }

						}
						#undef GENERR
						break;
					}
				}

				for (const auto& tkn : tokens) {
					ENGINE_RAW_TEXT("Token(", (int)tkn.type, "): |", tkn.view(data), "|\n");
				}

				if (err) {
					Index newlineCount = 0;
					Index errorIndex = i;
					constexpr auto prevLinesToDisplay = 2;

					while (i > 0) {
						if (isNewline() && (++newlineCount > prevLinesToDisplay)) {
							++i;
							break;
						};

						--i;
					}

					Token tkn;
					tkn.reset(i);
					i = errorIndex;
					while (++i, !isEOF()) {
						if (i > errorIndex && isNewline()) { --i; break; }
					}

					tkn.stop = i;

					ENGINE_WARN("Error parsing config\n",
						file, ":", line + 1, ":", i - lineStart + 1, ": ",
						err, area ? " in " : "", area ? area : "", '\n',
						std::string(80, '-'), '\n',
						tkn.view(data), '\n',
						std::string(std::max(0, errorIndex - lineStart), ' '), "^\n"
						//, std::string(80, '-'), '\n'
					);
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

			ENGINE_INLINE bool isSign() const noexcept {
				const auto c = data[i];
				return c == '-' || c == '+';
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

			bool eatBinInteger() {
				if (!isSpaceRemaining(3)) {
					err = "Invalid binary number";
					return false;
				}

				Token tkn;
				tkn.reset(i);
				if (isSign()) { ++i; }

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

			bool eatHexInteger() {
				if (!isSpaceRemaining(3)) {
					err = "Invalid hexadecimal number";
					return false;
				}

				Token tkn;
				tkn.reset(i);
				if (isSign()) { ++i; }

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

			bool eatDecInteger() {
				Token tkn;
				tkn.reset(i);
				if (isSign()) { ++i; }

				if (data[i] == '0') {
					err = "Decimal numbers may not have leading zeros";
					return false;
				}

				while (!isEOF() && isDigit()) { ++i; }
				tkn.stop = i - 1;

				if (tkn.size() <= 0) {
					err = "Invalid decimal digit";
					return false;
				}

				tkn.type = Token::Type::DecLiteral;
				tokens.push_back(tkn);
				return true;
			}

			bool eatBool() {
				Token tkn;
				tkn.reset(i);
				bool succ = true;

				if (isSpaceRemaining(5)) {
					for (const auto c : {'f', 'a', 'l', 's', 'e'}) {
						if (c != tolower(data[i])) { succ = false; break; }
						++i;
					}

				}

				if (!succ && isSpaceRemaining(4)) {
					i = tkn.start;
					succ = true;

					for (const auto c : {'t', 'r', 'u', 'e'}) {
						if (c != tolower(data[i])) { succ = false; break; }
						++i;
					}
				}

				if (succ) {
					tkn.stop = i - 1;
					tkn.type = Token::Type::BoolLiteral;
					tokens.push_back(tkn);
					return true;
				}

				err = "Invalid boolean value";
				return false;
			}

			bool eatDecFloat() {
				Token tkn;
				tkn.reset(i);

				if (isSign()) { ++i; }

				// Integer part
				if (isEOF() || !isDigit()) { err = "Expected digit"; return false; }
				++i;
				while (!isEOF() && isDigit()) { ++i; };

				// Fraction part
				if (!isEOF() && data[i] == '.') {
					++i;
					const auto fractStart = i;
					while (!isEOF() && isDigit()) { ++i; };
					if (i == fractStart) { err = "Expected fractional digit"; return false; }
				}

				// Exponent part
				if (!isEOF()) {
					if (const auto c = data[i]; c == 'e' || c == 'E' || c == 'p' || c == 'P') {
						++i;
						if (!isEOF() && isSign()) { ++i; }
						// TODO: exponent sign
						const auto expStart = i;
						while (!isEOF() && isDigit()) { ++i; };
						if (i == expStart) { err = "Expected exponent digit"; return false; }
					}
				}

				tkn.stop = i - 1;
				tkn.type = Token::Type::FloatLiteral;
				tokens.push_back(tkn);

				return true;
			}

			bool eatString() {
				if (data[i] != '"') { err = "Invalid quote character"; return false; }
				Token tkn;
				tkn.reset(i);

				constexpr const char* eofErr = "Reached end of file before finding string terminator";

				bool escaped = false;
				while (true) {
					++i;
					if (isEOF()) { err = eofErr; return false; }
					if (!escaped) {
						if (data[i] == '\\') { escaped = true; continue; }
						if (data[i] == '"') { break; }
					}
					if (isNewline()) {
						++line;
						lineStart = i + 1;
					}
					escaped = false;
				}

				tkn.stop = i;
				++i;
				tkn.type = Token::Type::StringLiteral;
				tokens.push_back(tkn);
				return true;
			}
			
			bool eatValue() {
				const Index pre = i;

				if (eatBinInteger()) { return true; }
				i = pre;
				err = nullptr;

				if (eatHexInteger()) { return true; }
				i = pre;
				err = nullptr;

				if (eatDecFloat()) { return true; }
				i = pre;
				err = nullptr;

				if (eatDecInteger()) { return true; }
				i = pre;
				err = nullptr;

				if (eatBool()) { return true; }
				i = pre;
				err = nullptr;
				
				const Index preLine = line;
				const Index preLineStart = lineStart;
				if (eatString()) { return true; }
				i = pre;
				line = preLine;
				lineStart = preLineStart;
				err = nullptr;


				err = "Unable to parse value";
				return false;
			}
	};
}
