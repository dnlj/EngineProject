#pragma once

namespace Engine {
	class BindPressListener {
		private:
			friend class Bind;
			virtual void onBindPress() = 0;
	};
}
