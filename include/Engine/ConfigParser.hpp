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
	class ConfigParser {
		private:
			using Index = int32;

			using Bool = bool;
			using Int = int64;
			using Float = float64;
			using String = std::string;

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
					StringLiteral,
					BinLiteral,
					HexLiteral,
					DecLiteral,
					FloatLiteral,
					BoolLiteral,
					_COUNT,
				};

				ENGINE_INLINE Type getType() const noexcept { return type; }

				template<class T>
				ENGINE_INLINE T& getData() noexcept { return data.as<T>(); }

				template<class T>
				ENGINE_INLINE const T& getData() const noexcept { return const_cast<Token*>(this)->getData<T>(); }

				ENGINE_INLINE static bool isStringType(Type t) noexcept { return (t <= Type::StringLiteral) && (t != Type::Unknown); }

				Token(Type t) { setType(t); }

				Token(Token&& other) { *this = std::move(other); }

				Token& operator=(Token&& other) {
					setType(other.type);
					if (isStringType(other.type)) {
						getData<String>() = std::move(other.getData<String>());
					} else {
						memcpy(&data, &other.data, sizeof(data));
					}
					return *this;
				}

				~Token() {
					if (isStringType(type)) { data.as<String>().~String(); }
				};

				private:
					void setType(Type t) {
						if (isStringType(t) && !isStringType(type)) { new (&data.as<String>()) String{}; }
						type = t;
					}

					union Data {
						public:
							Data() {}
							~Data() {}

							template<class T>
							T& as() {
								if constexpr (std::is_same_v<T, Bool>) { return asBool; }
								if constexpr (std::is_same_v<T, Int>) { return asInt; }
								if constexpr (std::is_same_v<T, Float>) { return asFloat; }
								if constexpr (std::is_same_v<T, String>) { return asString; }
							}

						private:
							Bool asBool;
							Int asInt;
							Float asFloat;
							String asString;
					};

					Type type = Type::Unknown;
					Data data;
			};

			template<class T>
			struct SetupStorageToken;

			template<std::integral T>
			struct SetupStorageToken<T> {
				using Type = Int;
				ENGINE_INLINE static void setup(Token& tkn) { tkn = Token::Type::DecLiteral; }
			};

			template<std::floating_point T>
			struct SetupStorageToken<T> {
				using Type = Float;
				ENGINE_INLINE static void setup(Token& tkn) { tkn = Token::Type::FloatLiteral; }
			};
			
			template<>
			struct SetupStorageToken<bool> {
				using Type = bool;
				ENGINE_INLINE static void setup(Token& tkn) { tkn = Token::Type::BoolLiteral; }
			};

			template<std::convertible_to<std::string> T>
			struct SetupStorageToken<T> {
				using Type = String;
				ENGINE_INLINE static void setup(Token& tkn) { tkn = Token::Type::StringLiteral; }
			};

		private:
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
			};

			FlatHashMap<std::string, KeyValuePair> keyLookup;
			FlatHashMap<std::string, Index> sectionLookup;

		public:
			std::string toString() const {
				std::string res;
				std::string sec;
				std::string key;
				std::string val;
				res.reserve(tokens.size() * 8); // Assume avg 8 chars per token
				val.reserve(128);

				for (const auto& t : tokens) {
					if (t.getType() == Token::Type::Key) {
						key = sec + t.getData<String>();
					} else if (t.getType() == Token::Type::Section) {
						const auto& str = t.getData<String>();
						if (str.empty()) {
							sec = "";
						} else {
							sec = {str.cbegin() + 1, str.cend() - 1};
							sec += ".";
						}
					}

					if (t.getType() > Token::Type::Assign) {
						const auto found = keyLookup.find(key);
						ENGINE_DEBUG_ASSERT(found != keyLookup.end());

						const auto& tkn = tokens[stable[found->second.value]];

						#define GEN(Enum, Type, Flag) { case Enum: StringConverter<Type>{}(tkn.getData<Type>(), val, Flag); break; }
						switch (t.getType()) {
							GEN(Token::Type::StringLiteral, String, StringFormatOptions::QuoteEscapeASCII);
							GEN(Token::Type::BinLiteral, Int, StringFormatOptions::BinInteger);
							GEN(Token::Type::HexLiteral, Int, StringFormatOptions::HexInteger);
							GEN(Token::Type::DecLiteral, Int, StringFormatOptions::DecInteger);
							GEN(Token::Type::FloatLiteral, Float, {});
							GEN(Token::Type::BoolLiteral, Bool, {});
							default: {
								ENGINE_WARN("Unknown token type. Skipping.");
								continue;
							}
						}
						res += val;
					} else {
						res += t.getData<String>();
					}
				}

				return res;
			}

			void print() {
				std::cout << "=================================================\n";
				std::cout << toString();
				std::cout << "=================================================\n";
			}

			void save(const std::string& path) const {
				std::fstream file{path, std::ios::out | std::ios::binary};

				if (!file) {
					ENGINE_WARN("Unable to write config to \"", path, "\"");
					return;
				}

				const auto& str = toString();
				file.write(str.data(), str.size());
			}

			template<class T>
			auto* get(const std::string& key) {
				using Stored = typename SetupStorageToken<T>::Type;
				// TODO: need to handle captialization while maintaining format
				const auto found = keyLookup.find(key);
				if (found == keyLookup.cend()) { return static_cast<Stored*>(nullptr); }
				auto& val = tokens[stable[found->second.value]].getData<Stored>();
				return &val;
			}

			template<class T>
			auto insert(const std::string& key, const T& value) -> typename SetupStorageToken<T>::Type* {
				auto found = keyLookup.find(key);
				if (found != keyLookup.cend()) { return nullptr; }

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
						insertAt = stable[secFound->second] + 1;
						const auto sz = tokens.size();
						for (; insertAt < sz; ++insertAt) {
							const auto& tkn = tokens[insertAt];
							if (tkn.getType() != Token::Type::Whitespace
								&& tkn.getType() != Token::Type::Comment) {
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
						addSection("[" + key.substr(0, last) + "]"); // TODO: should we append [] inside addSection?
						insertAt = static_cast<Index>(tokens.size());
					}
				}

				return &addPairAt(insertAt, key, shortKey, value);
			}

			void remove(const std::string& key) {
				const auto found = keyLookup.find(key);
				if (found == keyLookup.cend()) { return; }
				const auto start = stable[found->second.key];
				auto stop = start;

				for (; stop < tokens.size(); ++stop) {
					const auto& tkn = tokens[stop];
					if (tkn.getType() > Token::Type::Assign) {
						++stop; break;
					}
				}

				ENGINE_INFO("Remove: [", start, ", ", stop, ")");
				const auto count = stop - start;
				for (auto& idx : stable) {
					if (idx >= stop) { idx -= count; }
				}

				tokens.erase(tokens.cbegin() + start, tokens.cbegin() + stop);
				keyLookup.erase(found);
			}

			// TODO: private
			void addSection(std::string sec) {
				sectionLookup[sec] = static_cast<Index>(tokens.size() + 1);

				Token tkns[3] = {
					Token::Type::Whitespace,
					Token::Type::Section,
					Token::Type::Whitespace,
				};

				tkns[0].getData<String>() = "\n";
				tkns[1].getData<String>() = std::move(sec);
				tkns[2].getData<String>() = "\n";

				tokens.push_back(std::move(tkns[0]));
				tokens.push_back(std::move(tkns[1]));
				tokens.push_back(std::move(tkns[2]));
			}


			// TODO: private
			template<class T>
			auto& addPairAt(Index idx, const std::string& key, std::string shortKey, const T& value) {
				using Setup = SetupStorageToken<T>;
				using Stored = typename Setup::Type;

				Token tkns[6] = {
					Token::Type::Key,
					Token::Type::Whitespace,
					Token::Type::Assign,
					Token::Type::Whitespace,
					Token::Type::BinLiteral,
					Token::Type::Whitespace,
				};

				tkns[0].getData<String>() = std::move(shortKey);
				tkns[1].getData<String>() = " ";
				tkns[2].getData<String>() = "=";
				tkns[3].getData<String>() = " ";
				Setup::setup(tkns[4]);
				tkns[5].getData<String>() = "\n";

				for (auto& v : stable) {
					if (v > idx) { v += static_cast<Index>(std::size(tkns)); }
				}

				KeyValuePair pair;
				pair.key = static_cast<Index>(stable.size());
				pair.value = pair.key + 4;
				ENGINE_LOG("Add pair: ", pair.key, " ", pair.value);

				auto it = 4 + tokens.insert(
					tokens.begin() + idx,
					std::make_move_iterator(&tkns[0]),
					std::make_move_iterator(&tkns[0] + std::size(tkns))
				);
				stable.insert(stable.cend(), {idx + 0, idx + 1, idx + 2, idx + 3, idx + 4, idx + 5});
				keyLookup.emplace(key, std::move(pair));

				return it->getData<Stored>() = value;
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
						switch (tokens.back().getType()) {
							GENERR(StringLiteral);
							GENERR(BinLiteral);
							GENERR(HexLiteral);
							GENERR(DecLiteral);
							GENERR(FloatLiteral);
							GENERR(BoolLiteral);
							default: { err = "Unexpected symbols after value definition"; }

						}
						#undef GENERR
						break;
					}
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

						if (tkn.getType() == Token::Type::Section) {
							const auto& str = tkn.getData<String>();
							const auto ss = str.size();

							ENGINE_DEBUG_ASSERT(ss > 1); // By this point all sections should already be validated.

							section.reserve(ss - 1);
							section.assign(++str.cbegin(), --str.cend());

							// In case of duplicates we only store the last one
							sectionLookup[section] = i;

							if (ss > 2) { section += "."; }

						} else if (tkn.getType() == Token::Type::Key) {
							const auto& str = tkn.getData<String>();
							const auto key = section.empty() ? str : section + str;
							const auto [it, inserted] = keyLookup.emplace(key, KeyValuePair{
								.key = i,
							});

							if (!inserted) {
								ENGINE_WARN("Duplicate entry found for key \"", key, "\"");
							}

							last = it;
						} else if (tkn.getType() > Token::Type::Assign) {
							last->second.value = i;
						}
					}
				}

				data.clear();
				data.shrink_to_fit();
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
			 * Currently only tabs spaces are considered inline whitespace.
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
				Token tkn{Token::Type::Whitespace};
				tkn.getData<String>() = rng.string(data);
				tokens.push_back(std::move(tkn));
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
				Token tkn{Token::Type::Whitespace};
				tkn.getData<String>() = rng.string(data);
				tokens.push_back(std::move(tkn));
				return true;
			}

			bool eatComment() {
				if (data[i] != '#') { return false; }
				Range rng = i;
				while (!isEOF() && !isNewline()) { ++i; }
				rng.stop = i - 1;

				Token tkn{Token::Type::Comment};
				tkn.getData<String>() = rng.string(data);
				tokens.push_back(std::move(tkn));
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

				Token tkn{Token::Type::Section};
				tkn.getData<String>() = rng.string(data);
				tokens.push_back(std::move(tkn));
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

				Token tkn{Token::Type::Key};
				tkn.getData<String>() = rng.string(data);
				tokens.push_back(std::move(tkn));
				return true;
			}

			bool eatAssign() {
				if (data[i] != '=') {
					err = "Expected assignment operator";
					return false;
				}
				++i;
				Token tkn{Token::Type::Assign};
				tkn.getData<String>() = '=';
				tokens.push_back(std::move(tkn));
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

				Token tkn{Token::Type::BinLiteral};
				Int val;
				if (!StringConverter<Int>{}(rng.string(data), val)) { return false; }
				tkn.getData<Int>() = val;
				tokens.push_back(std::move(tkn));
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

				Token tkn{Token::Type::HexLiteral};
				Int val;
				if (!StringConverter<Int>{}(rng.string(data), val)) { return false; }
				tkn.getData<Int>() = val;
				tokens.push_back(std::move(tkn));
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

					Token tkn{Token::Type::BoolLiteral};
					Bool val;
					if (!StringConverter<Bool>{}(rng.string(data), val)) { return false; }
					tkn.getData<Bool>() = val;
					tokens.push_back(std::move(tkn));
					return true;
				}

				err = "Invalid boolean value";
				return false;
			}

			bool eatDecNumber() {
				Range rng = i;

				Token tkn{Token::Type::DecLiteral};

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
					tkn = Token{Token::Type::FloatLiteral};
				}

				// Exponent part
				if (!isEOF()) {
					if (const auto c = data[i]; c == 'e' || c == 'E' || c == 'p' || c == 'P') {
						++i;
						if (!isEOF() && isSign()) { ++i; }
						const auto expStart = i;
						while (!isEOF() && isDigit()) { ++i; };
						if (i == expStart) { err = "Expected exponent digit"; return false; }
						tkn = Token{Token::Type::FloatLiteral};
					}
				}

				rng.stop = i - 1;

				if (tkn.getType() == Token::Type::FloatLiteral) {
					Float val;
					if (!StringConverter<Float>{}(rng.string(data), val)) { return false; }
					tkn.getData<Float>() = val;
				} else {
					Int val;
					if (!StringConverter<Int>{}(rng.string(data), val)) { return false; }
					tkn.getData<Int>() = val;
				}

				tokens.push_back(std::move(tkn));
				return true;
			}

			bool eatString() {
				constexpr const char* eofErr = "Reached end of file before finding string terminator";

				if (data[i] != '"') { err = "Invalid quote character"; return false; }
				Range rng = i;

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

				Token tkn{Token::Type::StringLiteral};
				tkn.getData<String>() = rng.string(data);
				tokens.push_back(std::move(tkn));
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
