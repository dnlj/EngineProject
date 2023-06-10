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

				// TODO: replace \q with q
				args.emplace_back(last+1, cur);
				++cur; // Move past the closing quote
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
