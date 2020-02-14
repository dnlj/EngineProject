#pragma once

namespace Engine::Glue {
	namespace _impl {
		template<class To, class From>
		struct as;
	}

	template<class To, class From>
	auto&& as(From&& v) {
		// TODO: is there a good way to get better error messages here?
		return _impl::as<To, From>::call(std::forward<From>(v));
	};
}
