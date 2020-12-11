#pragma once

// STD
#include <string>
#include <vector>
#include <memory>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/CommandLine/Argument.hpp>


namespace Engine::CommandLine {
	class Parser {
		private:
			FlatHashMap<std::string, std::unique_ptr<detail::ArgumentBase>> params;
			FlatHashMap<char, detail::ArgumentBase*> abbrToFull;
			std::vector<std::string> positional;

		public:
			void parse(int argc, char* argv[]);
			
			template<class T>
			Parser& add(std::string full, std::string help) {
				return add<T>(std::move(full), {}, {}, std::move(help), false);
			};

			template<class T>
			Parser& add(std::string full, T def, std::string help) {
				return add<T>(std::move(full), 0, std::move(def), std::move(help));
			};

			template<class T>
			Parser& add(std::string full, char abbr, T def, std::string help, bool useDefault = true);

			template<class T>
			const T* get(const std::string& full) const;
	};
}

#include <Engine/CommandLine/Parser.ipp>
