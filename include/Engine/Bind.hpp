#pragma once

// STD
#include <string>
#include <vector>
#include <type_traits>

// Engine
#include <Engine/BindListener.hpp>
#include <Engine/BindPressListener.hpp>
#include <Engine/BindReleaseListener.hpp>


// TODO: Doc
namespace Engine {
	class Bind {
		public:
			Bind(std::string name);

			void press();

			void release();

			// TODO handle hold listeners? or should we have the listeners implement that themselves?

			// TODO: Will also need an BindChangeListener for axis inputs.
			// TODO: How should axis inputs work with press/release listeners? zero/nonzero? epsilon? Are deadzones handles by drivers?
			// TODO: Could replace both addListener functions with a templated addListener method that does the correct thing in the case of multiple inheritance if we wanted to.
			void addPressListener(BindPressListener* listener);
			void addReleaseListener(BindReleaseListener* listener);
			// TODO: Do we want an on changed listener?

			/**
			 * Register a BindListener for all applicable events based on parent types.
			 * @param listener The listener to register.
			 * @tparam Listener The type of the listener.
			 */
			template<class Listener, class = std::enable_if_t<std::is_base_of_v<BindListener, Listener>>>
			void addListener(Listener* listener);

			// TODO: removeListener functions

			const std::string name;

		private:
			int active = 0;
			std::vector<BindPressListener*> pressListeners;
			std::vector<BindReleaseListener*> releaseListeners;
	};
}

#include <Engine/Bind.ipp>
