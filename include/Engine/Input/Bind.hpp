#pragma once

// STD
#include <string>
#include <vector>
#include <type_traits>

// Engine
#include <Engine/Input/BindListener.hpp>
#include <Engine/Input/BindPressListener.hpp>
#include <Engine/Input/BindHoldListener.hpp>
#include <Engine/Input/BindReleaseListener.hpp>


// TODO: Doc
namespace Engine::Input {
	class Bind {
		public:
			Bind(std::string name);

			void press();
			void hold() const;
			void release();

			/**
			 * Checks if a bind has been pressed and not released.
			 */
			bool isActive() const;

			// TODO: Will also need an BindChangeListener for axis inputs.
			// TODO: How should axis inputs work with press/release listeners? zero/nonzero? epsilon? Are deadzones handles by drivers?
			void addPressListener(BindPressListener* listener);
			void addHoldListener(BindHoldListener* listener);
			void addReleaseListener(BindReleaseListener* listener);

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
			std::vector<BindHoldListener*> holdListeners;
			std::vector<BindReleaseListener*> releaseListeners;
	};
}

#include <Engine/Input/Bind.ipp>
