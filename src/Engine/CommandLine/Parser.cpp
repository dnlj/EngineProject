// Engine
#include <Engine/Engine.hpp>
#include <Engine/CommandLine/Parser.hpp>

namespace Engine::CommandLine {
	// TODO: positional arg types?
	void Parser::parse(int argc, char* argv[]) {
		std::vector<std::string> args;
		args.reserve(2ll * argc);

		for (int i = 0; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == "--") {
				for (auto j = i + 1; j < argc; ++j) {
					args.push_back(argv[j]);
				}
				break;
			}

			auto found = arg.find('=');
			if (found == std::string::npos) {
				if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
					for (int i = 1; i < arg.size(); ++i) {
						args.emplace_back("-") += arg[i];
					}
				} else {
					args.push_back(arg);
				}
			} else {
				args.push_back(arg.substr(0, found));
				args.push_back(arg.substr(found + 1));
			}
		}

		const int last = static_cast<int>(args.size()) - 1;
		for (int i = 0; i <= last; ++i) {
			auto& arg = args[i];
			detail::ArgumentBase* ptr = nullptr;

			// Explicit positional arguments
			if (arg == "--") {
				for (int j = i + 1; j <= last; ++j) {
					positional.emplace_back(std::move(args[j]));
				}
				break;
			}

			if (arg.starts_with("--")) { // Full arguments
				const auto full = arg.substr(2);
				auto found = params.find(full);
				if (found == params.end()) {
					ENGINE_WARN("Unknown command line argument: ", arg);
					found = params.emplace(full, std::make_unique<Argument<std::string>>(0, "", "<invalid argument>", true)).first;
				}
				ptr = found->second.get();
			} else if (arg.starts_with("-")) { // Abrev aguments
				auto found = abbrToFull.find(arg[1]);
				if (found == abbrToFull.end()) {
					ENGINE_WARN("Unkown abbreviated command line argument: ", arg);
					const auto [it, _] = params.emplace("<invalid> " + arg, std::make_unique<Argument<std::string>>(0, "", "<invalid argument>", true));
					found = abbrToFull.emplace(arg[1], it->second.get()).first;
				}
				ptr = found->second;
			} else { // Implicit positional arguments
				positional.emplace_back(std::move(arg));
				continue;
			}

			// Store the value if there is one. I there isnt one assume it is a boolean flag argument.
			if ((i < last) && !args[i+1].starts_with("-")) {
				ptr->store(args[++i]);
			} else {
				ptr->store("1");
			}
		}
	}
}
