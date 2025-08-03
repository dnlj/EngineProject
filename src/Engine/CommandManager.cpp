// Engine
#include <Engine/CommandManager.hpp>
#include <Engine/Unicode/UTF8.hpp>


namespace Engine {
	CommandManager::CommandManager() {
		[[maybe_unused]] const auto id = registerCommandUnchecked("$_INVALID", nullptr);
		ENGINE_DEBUG_ASSERT(id == CommandId::Invalid);
	}
	
	void CommandManager::exec(CommandId id) {
		ENGINE_DEBUG_ASSERT(+id < commands.size(), "Attempting to execute invalid command.");

		if (id == CommandId::Invalid) {
			ENGINE_WARN2("Invalid command id");
			arguments.clear();
			return;
		}

		auto& cmd = commands[+id];
		cmd.func(*this);
		arguments.clear();
	}

	void CommandManager::exec(std::string_view str) {
		parse(str);

		if (arguments.empty()) {
			ENGINE_WARN2("Invalid command: {}", str);
			return;
		}

		const auto& name = arguments.front();
		const auto id = lookup(name);
		exec(id);
	}

	void CommandManager::parse(std::string_view str) {
		ENGINE_DEBUG_ASSERT(arguments.empty(), "Arguments were not cleared by last exec.");

		const auto end = std::to_address(str.cend());
		auto cur = std::to_address(str.cbegin());
		auto last = cur;

		const auto isQuote = [&]() -> char ENGINE_INLINE {
			if (*cur == '"' || *cur == '\'') { return *cur; }
			return 0;
		};

		const auto isSpace = [&]() ENGINE_INLINE {
			return *cur == ' ' || *cur == '\t';
		};

		const auto eatQuote = [&]() ENGINE_INLINE -> char {
			const auto q = isQuote();
			if (!q) { return false; }

			ENGINE_DEBUG_ASSERT(last == cur);
			while (++cur != end) {
				if (*cur == q && *(cur-1) != '\\') { return q; }
			}

			// Hit end of string without closing quote.
			return q;
		};

		const auto eatSpace = [&]() ENGINE_INLINE {
			if (isSpace()) {
				while (++cur != end) {
					if (!isSpace()) { break; }
				}
				return true;
			}
			return false;
		};
		
		const auto eatWord = [&]() ENGINE_INLINE {
			if (!isSpace()) {
				while (++cur != end) {
					if (isSpace()) { break; }
				}
				return true;
			}
			return false;
		};

		// Leading space
		eatSpace();
		last = cur;

		// Main parsing
		while (cur != end) {
			if (const auto q = eatQuote()) {
				if (cur == end) {
					ENGINE_WARN("Missing closing quotation mark.");
					break;
				}

				// Skip leading quote
				++last;

				// Replace \q with q and copy to arguments
				const auto fend = cur;
				auto fcur = last;
				auto& arg = arguments.emplace_back();
				arg.reserve(cur - last);

				while (true) {
					auto found = std::find(fcur, fend, '\\');
					if (found == fend || ++found == fend) { break; }
					if (*found == q || *found == '\\') {
						arg.append(fcur, found); // Insert the current range including the escape "\"
						arg.back() = *found; // Replace the escape "\" with the escaped char

						// Skip the escaped character next pass. We need to do things this
						// way so we can support escaping a backslash "\\".
						fcur = found + 1;
					}
				}

				if (fcur != fend) {
					arg.append(fcur, fend);
				}

				// Move past trailing quote
				++cur;
			} else if (eatWord()) {
				arguments.emplace_back(last, cur);
			} else {
				ENGINE_WARN("Expected argument.");
				break;
			}

			if (cur == end) { break; }

			if (!eatSpace()) {
				ENGINE_WARN("Expected space.");
				break;
			}

			last = cur;
		}
	}
}
