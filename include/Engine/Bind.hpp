#pragma once

// STD
#include <string>
#include <vector>


// TODO: Doc
// TODO: Split
namespace Engine {
	class BindPressListener {
		public:
			virtual void onBindPress() = 0;
	};

	class BindReleaseListener {
		public:
			virtual void onBindRelease() = 0;
	};

	class Bind {
		public:
			Bind(std::string name) : name{std::move(name)} {
			}

			void press() {
				if (active == 0) {
					for (auto l : pressListeners) {
						// TODO: call press listeners
						l->onBindPress();
					}
				}

				++active;
			};

			void release() {
				--active;

				if (active == 0) {
					for (auto l : releaseListeners) {
						// TODO: release listeners
						l->onBindRelease();
					}
				}
			};

			// TODO handle hold listeners? or should we have the listeners implement that themselves?

			void addPressListener(BindPressListener* listener) {
				pressListeners.push_back(listener);
			}

			void addReleaseListener(BindReleaseListener* listener) {
				releaseListeners.push_back(listener);
			}

			const std::string name;

		private:
			int active = 0;
			std::vector<BindPressListener*> pressListeners;
			std::vector<BindReleaseListener*> releaseListeners;
	};
}
