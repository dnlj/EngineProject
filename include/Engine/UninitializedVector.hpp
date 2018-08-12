#pragma once

// STD
#include <cstdlib>


// TODO: Doc
// TODO: Test
namespace Engine {
	template<class T>
	class UninitializedVector {
		public:
			~UninitializedVector();
			std::size_t size() const;

		private:
			size_t count = 0;
			void* data = nullptr;
	};
}

#include <Engine/UninitializedVector.ipp>
