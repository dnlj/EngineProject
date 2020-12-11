#pragma once

// Engine
#include <Engine/CommandLine/Parser.hpp>


namespace Engine::CommandLine {
	template<class T>
	auto Parser::add(std::string full, char abbr, T def, std::string help, bool useDefault) -> Parser& {
		if (abbr == '-') {
			abbr = 0;
			ENGINE_WARN("Command line abbreviation for '", full, "' may not be the character '-'. Ignoring.");
		}

		if (full.find(' ') != full.npos) {
			ENGINE_WARN("The command line argument '", full, "' may not contain spaces. Ignoring.");
			return *this;
		}

		if (params.find(full) != params.end()) {
			ENGINE_WARN("Duplicate command line argument added '", full, "'. Ignoring.");
			return *this;
		}

		if (abbr != 0  && abbrToFull.find(abbr) != abbrToFull.end()) {
			ENGINE_WARN("Duplicate abbreviated command line argument added '", abbr, "'. Ignoring.");
			return *this;
		}

		const auto [it, _] = params.emplace(std::move(full), std::make_unique<Argument<T>>(abbr, std::move(def), std::move(help)));
		if (abbr != 0) {
			abbrToFull.emplace(abbr, it->second.get());
		}

		if (useDefault) {
			it->second->setHasBeenSet(true);
		}

		return *this;
	}

	template<class T>
	const T* Parser::get(const std::string& full) const {
		const auto found = params.find(full);

		if (found == params.end()) {
			ENGINE_WARN("Attempting to get invalid command line argument '", full, "'");
			return nullptr;
		}

		if (detail::getTypeId<T>() != found->second->typeId) {
			ENGINE_WARN("Attempting to get command line argument with incorrect type '", full, "'");
			return nullptr;
		}

		if (!found->second->hasBeenSet()) {
			return nullptr;
		}

		return static_cast<const T*>(found->second->get());
	}
}
