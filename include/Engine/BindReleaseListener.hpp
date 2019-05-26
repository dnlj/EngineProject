#pragma once


namespace Engine {
	class BindReleaseListener {
		private:
			friend class Bind;
			virtual void onBindRelease() = 0;
	};
}
