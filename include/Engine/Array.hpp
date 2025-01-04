#include <array>

namespace Engine {
	// TODO: aliaases and helpers for 
	//template<class T, auto... Dims>
	//struct ArrayType;
	//
	//template<class T>
	//struct ArrayType<T>{};
	//
	//template<class T, size_t Size0>
	//struct ArrayType<T, Size0> {
	//	using Type = std::array<T, Size0>;
	//};
	//
	// This isn't quite right... it doens't unwrap at all
	//template<class T, size_t Size0, size_t Size1, auto... SizeN>
	//struct ArrayType<T, Size0, Size1, SizeN...> {
	//	using Type = ArrayType<ArrayType<T, Size0>::Type, Size1>::Type;
	//};
	//
	//template<class T, size_t X, auto... Args>
	//using Array = std::array<T, X>;

	//template<class T, size_t X, size_t Y>
	//using Array<T, X, Y, 0> = Array<Array<T, Y>, X>;

	//template<class T, size_t X, size_t Y, size_t Z>
	//using Array = Array<Array<Array<T, Z>, Y>, X>;

	// TODO: MakeArray(init value)
}
