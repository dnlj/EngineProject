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
		if (*refCount == 1) {
			delete refCount;
			std::terminate();
		}
	}
}