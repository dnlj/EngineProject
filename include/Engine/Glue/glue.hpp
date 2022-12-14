#pragma once

namespace Engine::Glue {
	namespace _impl {
		template<class To, class From>
		struct as {
			static_assert(ENGINE_TMP_FALSE(To), "Unknown Engine::Glue::as conversion. Did you include the correct headers?");
		};
	}

	template<class To, class From>
	decltype(auto) as(From&& v) {
		// TODO: is there a good way to get better error messages here?
		// TODO: why do we pass From here? we dont use it.
		return _impl::as<To, From>::call(std::forward<From>(v));
	};
}
