// Engine
#include <Engine/CommandManager.hpp>
#include <Engine/Unicode/UTF8.hpp>


namespace Engine {
	void CommandManager::exec(std::string_view str) {
		// TODO: Need to:
		// * Parse into components
		// * Validate and find command meta
		// * Setup command arguments
		// * Call exec(meta.cid);
		args.clear();
		const auto end = std::to_address(str.cend());
		auto cur = std::to_address(str.cbegin());
		auto last = cur;

		// TODO: cur != end;

		const auto isQuote = [&]() -> char ENGINE_INLINE {
			if (*cur == '"' || *cur == '\'') { return *cur; }
			return 0;
		};

		const auto isSpace = [&]() -> bool ENGINE_INLINE {
			return *cur == ' ' || *cur == '\t';
		};

		const auto eatQuote = [&]() ENGINE_INLINE -> char {
			const auto q = isQuote();
			if (!q) { return false; }
			ENGINE_DEBUG_ASSERT(last == cur);
			while (++cur != end) {
				if (*cur == q && *(cur-1) != '\\') { return q; }
			}

			// TODO: error
			// Hit end of string without closing quote.
			return q;
		};

		const auto eatSpace = [&]() ENGINE_INLINE -> bool {
			if (isSpace()) {
				while (++cur != end) {
					if (!isSpace()) { break; }
				}
				return true;
			}
			return false;
		};
		
		const auto eatWord = [&]() ENGINE_INLINE -> bool {
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

		// TODO: remove
		ENGINE_INFO("Command: ", str);

		// Main parsing
		while (cur != end) {
			if (const auto q = eatQuote()) {
				if (cur == end) {
					ENGINE_WARN("Missing closing quotation mark.");
					break;
				}

				// Skip leading quote
				++last;

				// Replace \q with q and copy to args
				const auto fend = cur;
				auto fcur = last;
				auto& arg = args.emplace_back();
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
				args.emplace_back(last, cur);
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

		for (const auto& arg : args) {
			ENGINE_INFO("Arg: ", arg);
		}
	}
}
