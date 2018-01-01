// STD
#include <exception>

// Engine
#include <Engine/FatalException.hpp>

namespace Engine {
	FatalException::FatalException() {
		refCount = new int{1};
	}

	FatalException::FatalException(const FatalException& other) {
		refCount = other.refCount;
		++*refCount;
	};

	FatalException::~FatalException() {
		--*refCount;

		if (*refCount == 0) {
			delete refCount;
			std::terminate();
		}
	}
}