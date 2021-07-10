#pragma once


namespace Engine {
	/**
	 * Thrown on an unrecoverable error.
	 * Calls std::terminate on destruction.
	 */
	class FatalException {
		public:
			FatalException();
			FatalException(const FatalException& other);
			~FatalException();

			FatalException& operator=(const FatalException& other) = delete;

		private:
			int* refCount = nullptr;
	};
}
