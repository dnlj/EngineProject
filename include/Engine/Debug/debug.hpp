#pragma once


namespace Engine::Debug {
	template<class T>
	std::string_view ClassName() {
		std::string_view name = typeid(T).name();
		return name.substr(name.find_last_of(':') + 1);
	}
}
