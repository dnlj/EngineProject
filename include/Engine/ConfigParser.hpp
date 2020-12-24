#pragma once

// STD
#include <string>
#include <fstream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/StringConverter.hpp>


namespace Engine {
	class AbstractGenericValue {
		public:
			virtual ~AbstractGenericValue() {}
			virtual bool set(std::string_view str) = 0;
			virtual std::string get() const = 0;

			template<class T>
			T& getValue() { return *reinterpret_cast<T*>(stored()); }

			template<class T>
			const T& getValue() const { return *reinterpret_cast<const T*>(stored()); }

		private:
			virtual void* stored() = 0;

	};
	
	template<class T, class Converter = StringConverter<T>>
	class GenericValueStore final : public AbstractGenericValue {
		private:
			T value;
			virtual void* stored() { return &value; }

		public:
			GenericValueStore() = default;
			GenericValueStore(std::string_view str) {
				set(str);
			}

			virtual bool set(std::string_view str) override {
				return Converter{}(str, value);
			}

			virtual std::string get() const override {
				std::string str;
				Converter{}(value, str);
				return str;
			}
	};

	class ConfigParser {
		private:
			using Index = int32;

			struct Range {
				Index start;
				Index stop;
				ENGINE_INLINE Range(Index i) { reset(i); }
				ENGINE_INLINE Index size() const noexcept { return stop - start + 1; }
				ENGINE_INLINE void reset(Index i) noexcept { start = i; stop = i - 1; }
				ENGINE_INLINE std::string_view view(const std::string& data) const { return std::string_view(&data[start], size()); };
				ENGINE_INLINE std::string string(const std::string data) const { return std::string{&data[start], &data[stop] + 1}; }
			};

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
				std::string data;
				Type type;
			};

		private:
			// TODO: maybe ptr? once we are done loading the file we dont need this second copy;
			std::string data;
			Index size;
			Index i;
			Index line;
			Index lineStart;
			const char* err = nullptr;

			std::vector<Token> tokens;

			/** Provides a stable reference to strings in @ref tokens */
			std::vector<Index> stable;

			struct KeyValuePair {
				Index key = -1;
				Index value = -1;
				std::unique_ptr<AbstractGenericValue> store;
			};

			FlatHashMap<std::string, KeyValuePair> keyLookup;
			FlatHashMap<std::string, Index> sectionLookup;

		public:
			void print() const {
				std::cout << "=================================================\n";
				std::string sec;
				std::string key;
				for (const auto& t : tokens) {
					if (t.type == Token::Type::Key) {
						key = sec + t.data;
					} else if (t.type == Token::Type::Section) {
						if (t.data.empty()) {
							sec = "";
						} else {
							sec = {t.data.begin() + 1, t.data.end() - 1};
							sec += ".";
						}
					}

					if (t.type > Token::Type::Assign) {
						const auto found = keyLookup.find(key);
						ENGINE_DEBUG_ASSERT(found != keyLookup.end());
						std::cout << found->second.store->get();
					} else {
						std::cout << t.data;
					}
				}
				std::cout << "=================================================\n";
			}

			template<class T>
			T* get(const std::string& key) {
				// TODO: need to handle captialization while maintaining format
				const auto found = keyLookup.find(key);
				if (found == keyLookup.cend()) { return nullptr; }
				return &found->second.store->getValue<T>();
			}

			template<class T>
			T& insert(const std::string& key, const T& value) {
				auto found = keyLookup.find(key);
				if (found == keyLookup.cend()) {
					auto last = std::string::npos;
					Index insertAt = -1;
					std::string shortKey;

					while ((last = key.rfind('.', last)) != std::string::npos) {
						// TODO: need to handle captialization while maintaining format
						const auto& sec = key.substr(0, last);
						shortKey = {key.cbegin() + last + 1, key.cend()};
						--last;

						ENGINE_LOG("Checking [", sec ,"] ", shortKey);
						const auto secFound = sectionLookup.find(sec);
						if (secFound != sectionLookup.cend()) {
							ENGINE_LOG("Found!");
							insertAt = secFound->second + 1;
							const auto sz = tokens.size();
							for (; insertAt < sz; ++insertAt) {
								const auto& tkn = tokens[insertAt];
								if (tkn.type != Token::Type::Whitespace
									&& tkn.type != Token::Type::Comment) {
									break;
								}
							}
							break;
						}
					}

					if (last == std::string::npos) {
						//found = keyLookup.insert(key, {}).first;
						last = key.rfind('.');
						if (last == std::string::npos) {
							// No section. Add to start of file.
							shortKey = key;
							insertAt = 0;
						} else {
							addSection(key.substr(0, last));
							insertAt = static_cast<Index>(tokens.size());
						}
					}

					// TODO: check that shortKey is valid
					ENGINE_INFO(shortKey.size());

					return addPairAt(insertAt, key, shortKey, value);
				}

				return found->second.store->getValue<T>();
			}

			// TODO: private
			void addSection(std::string sec) {
				tokens.emplace_back("\n", Token::Type::Whitespace);
				sectionLookup[sec] = static_cast<Index>(tokens.size());
				tokens.emplace_back(std::move(sec), Token::Type::Section);
				tokens.emplace_back("\n", Token::Type::Whitespace);
			}

			// TODO: private
			template<class T>
			T& addPairAt(Index idx, const std::string& key, std::string shortKey, const T& value) {
				Token tkns[6];

				tkns[0].data = std::move(shortKey);
				tkns[0].type = Token::Type::Key;

				tkns[1].data = " ";
				tkns[1].type = Token::Type::Whitespace;

				tkns[2].data = "=";
				tkns[2].type = Token::Type::Assign;

				tkns[3].data = " ";
				tkns[3].type = Token::Type::Whitespace;

				// tkns[4] is handled below

				tkns[5].data = "\n";
				tkns[5].type = Token::Type::Whitespace;

				// TODO: should probably convert T to int64/float64 here what if store int32 then get int64. Need stable types
				if constexpr (std::is_same_v<T, bool>) {
					tkns[4].type = Token::Type::BoolLiteral;
				} else if constexpr (std::is_integral_v<T>) {
					tkns[4].type = Token::Type::DecLiteral;
				} else if constexpr (std::is_floating_point_v<T>) {
					tkns[4].type = Token::Type::FloatLiteral;
				} else if constexpr (std::is_convertible_v<T, std::string>) { // TODO: should i also check is_constructible? is_assignable?
					// TODO: test with char*
					// TODO: test with std::string
					// TODO: test with string_view
					tkns[4].type = Token::Type::StringLiteral;
				}

				for (auto& v : stable) {
					if (v > idx) { v += static_cast<Index>(std::ssize(tkns)); }
				}

				KeyValuePair pair;
				pair.store = std::make_unique<GenericValueStore<T>>();
				pair.store->getValue<T>() = value;
				pair.key = static_cast<Index>(stable.size());
				pair.value = pair.key + 4;

				tokens.insert(tokens.begin() + idx, &tkns[0], &tkns[0] + std::size(tkns));
				stable.insert(stable.cend(), {idx + 0, idx + 1, idx + 2, idx + 3, idx + 4, idx + 5});

				auto [it, _] = keyLookup.emplace(key, std::move(pair));

				return it->second.store->getValue<T>();
			}

			void loadAndTokenize(const std::string& file) {
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

				// TODO: rm
				//for (const auto& tkn : tokens) {
				//	ENGINE_RAW_TEXT("Token(", (int)tkn.type, "): |", tkn.view(data), "|\n");
				//}

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

					Range rng = i;
					i = errorIndex;
					while (++i, !isEOF()) {
						if (i > errorIndex && isNewline()) { --i; break; }
					}

					rng.stop = i;

					ENGINE_WARN("Error parsing config\n",
						file, ":", line + 1, ":", i - lineStart + 1, ": ",
						err, area ? " in " : "", area ? area : "", '\n',
						std::string(80, '-'), '\n',
						rng.view(data), '\n',
						std::string(std::max(0, errorIndex - lineStart), ' '), "^\n"
						//, std::string(80, '-'), '\n'
					);
				} else {
					ENGINE_INFO("Parsed with no errors and ", tokens.size(), " tokens");

					// TODO: move into own function?
					std::string section;
					const auto sz = tokens.size();
					decltype(keyLookup)::iterator last;
					for (Index i = 0; i < sz; ++i) {
						const auto& tkn = tokens[i];
						stable.push_back(i);

						if (tkn.type == Token::Type::Section) {
							const auto ss = tkn.data.size();

							ENGINE_DEBUG_ASSERT(ss > 1); // By this point all sections should already be validated.

							section.reserve(ss - 1);
							section.assign(++tkn.data.cbegin(), --tkn.data.cend());

							// In case of duplicates we only store the last one
							sectionLookup[section] = i;

							if (ss > 2) { section += "."; }

						} else if (tkn.type == Token::Type::Key) {
							const auto key = section.empty() ? tkn.data : section + tkn.data;
							const auto [it, inserted] = keyLookup.emplace(key, KeyValuePair{
								.key = i,
							});

							if (!inserted) {
								ENGINE_WARN("Duplicate entry found for key \"", key, "\"");
							}

							last = it;
						} else if (tkn.type > Token::Type::Assign) {
							last->second.value = i;

							#define GEN(Enum, Type) case Enum: { last->second.store = std::make_unique<GenericValueStore<Type>>(tkn.data); break; }
							switch (tkn.type) {
								GEN(Token::Type::BinLiteral, int64)
								GEN(Token::Type::HexLiteral, int64)
								GEN(Token::Type::DecLiteral, int64)
								GEN(Token::Type::FloatLiteral, float64)
								GEN(Token::Type::BoolLiteral, bool)
								GEN(Token::Type::StringLiteral, std::string)
								default: {
									ENGINE_ERROR("Unknown value type: ", static_cast<int>(tkn.type));
								}
							}
							#undef GEN
						}
					}
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
				Range rng = i;

				while (!isEOF() && isInlineWhitespace()) { ++i; }
				rng.stop = i - 1;

				if (rng.size() <= 0) { return false; }
				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::Whitespace;
				tokens.push_back(tkn);
				return true;
			}

			bool eatWhitespace() {
				Range rng = i;

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
				rng.stop = i - 1;

				if (rng.size() <= 0) { return false; }
				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::Whitespace;
				tokens.push_back(tkn);
				return true;
			}

			bool eatComment() {
				if (data[i] != '#') { return false; }
				Range rng = i;
				while (!isEOF() && !isNewline()) { ++i; }
				rng.stop = i - 1;

				Token tkn;
				tkn.data = rng.string(data);
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
				Range rng = i;
				while (++i) {
					if (isEOF()) { err = "Unexpected end of file"; return false; }
					if (data[i] == ']') { break; }
					if (isNewline()) { err = "Unexpected new line"; return false; }
				}
				rng.stop = i;

				if (rng.size() < 2) {
					err = "Incomplete section";
					return false;
				}

				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::Section;
				tokens.push_back(tkn);
				++i;
				return true;
			}

			bool eatKey() {
				Range rng = i;

				while (!isWhitespace() && !isEOF()) { ++i; }
				rng.stop = i - 1;
				if (rng.size() <= 0) {
					err = "No key name given";
					return false;
				}

				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::Key;
				tokens.push_back(tkn);
				return true;
			}

			bool eatAssign() {
				if (data[i] != '=') {
					err = "Expected assignment operator";
					return false;
				}
				++i;
				Token tkn;
				tkn.data = '=';
				tkn.type = Token::Type::Assign;
				tokens.push_back(tkn);
				return true;
			}

			bool eatBinInteger() {
				if (!isSpaceRemaining(3)) {
					err = "Invalid binary number";
					return false;
				}

				Range rng = i;
				if (isSign()) { ++i; }

				if (data[i] != '0' || (data[++i] != 'b' && data[i] != 'B')) {
					err = "binary numbers must have format 0b????";
					return false;
				}

				while (++i, !isEOF() && !isWhitespace()) {
					if (!isBinaryDigit()) { break; }
				}
				rng.stop = i - 1;

				if (rng.size() < 3) {
					err = "Invalid binary digit";
					return false;
				}

				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::BinLiteral;
				tokens.push_back(tkn);
				return true;
			}

			bool eatHexInteger() {
				if (!isSpaceRemaining(3)) {
					err = "Invalid hexadecimal number";
					return false;
				}

				Range rng = i;
				if (isSign()) { ++i; }

				if (data[i] != '0' || (data[++i] != 'x' && data[i] != 'X')) {
					err = "hexadecimal numbers must have format 0x????";
					return false;
				}

				while (++i, !isEOF() && !isWhitespace()) {
					if (!isHexDigit()) { break; }
				}
				rng.stop = i - 1;

				if (rng.size() < 3) {
					err = "Invalid binary digit";
					return false;
				}

				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::HexLiteral;
				tokens.push_back(tkn);
				return true;
			}

			bool eatDecInteger() {
				Range rng = i;
				if (isSign()) { ++i; }

				if (data[i] == '0') {
					err = "Decimal numbers may not have leading zeros";
					return false;
				}

				while (!isEOF() && isDigit()) { ++i; }
				rng.stop = i - 1;

				if (rng.size() <= 0) {
					err = "Invalid decimal digit";
					return false;
				}

				Token tkn;
				tkn.data = rng.string(data);
				tkn.type = Token::Type::DecLiteral;
				tokens.push_back(tkn);
				return true;
			}

			bool eatBool() {
				Range rng = i;
				bool succ = true;

				if (isSpaceRemaining(5)) {
					for (const auto c : {'f', 'a', 'l', 's', 'e'}) {
						if (c != tolower(data[i])) { succ = false; break; }
						++i;
					}

				}

				if (!succ && isSpaceRemaining(4)) {
					i = rng.start;
					succ = true;

					for (const auto c : {'t', 'r', 'u', 'e'}) {
						if (c != tolower(data[i])) { succ = false; break; }
						++i;
					}
				}

				if (succ) {
					rng.stop = i - 1;

					Token tkn;
					tkn.data = rng.string(data);
					tkn.type = Token::Type::BoolLiteral;
					tokens.push_back(tkn);
					return true;
				}

				err = "Invalid boolean value";
				return false;
			}

			bool eatDecNumber() {
				Range rng = i;

				Token tkn;
				tkn.type = Token::Type::DecLiteral;

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
					tkn.type = Token::Type::FloatLiteral;
				}

				// Exponent part
				if (!isEOF()) {
					if (const auto c = data[i]; c == 'e' || c == 'E' || c == 'p' || c == 'P') {
						++i;
						if (!isEOF() && isSign()) { ++i; }
						const auto expStart = i;
						while (!isEOF() && isDigit()) { ++i; };
						if (i == expStart) { err = "Expected exponent digit"; return false; }
						tkn.type = Token::Type::FloatLiteral;
					}
				}

				rng.stop = i - 1;
				tkn.data = rng.string(data);
				tokens.push_back(tkn);

				return true;
			}

			bool eatString() {
				if (data[i] != '"') { err = "Invalid quote character"; return false; }
				Range rng = i;

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

				rng.stop = i;
				++i;

				Token tkn;
				tkn.data = rng.string(data);
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
				
				if (eatDecNumber()) { return true; }
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
