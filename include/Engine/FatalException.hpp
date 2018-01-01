#pragma once

// TODO: Doc
namespace Engine {
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