#pragma once

// STD
#include <string_view>
#include <ostream>

// TODO: Doc
namespace Engine::Detail {
	std::ostream& log(std::ostream& out, std::string_view prefix, std::string_view file, int line);
}