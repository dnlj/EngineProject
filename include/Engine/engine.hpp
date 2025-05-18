#pragma once

#define ENGINE_SIDE_SERVER 1
#define ENGINE_SIDE_CLIENT 2
#define ENGINE_SIDE_BOTH (ENGINE_SIDE_SERVER | ENGINE_SIDE_CLIENT)
#define ENGINE_SERVER (ENGINE_SIDE == ENGINE_SIDE_SERVER)
#define ENGINE_CLIENT (ENGINE_SIDE == ENGINE_SIDE_CLIENT)

#define ENGINE_DISABLE_ALL_LOGGING false

#ifdef DEBUG
	#define ENGINE_DEBUG true
	#define ENGINE_DEBUG_ONLY(...) __VA_ARGS__
#else
	#define ENGINE_DEBUG false
	#define ENGINE_DEBUG_ONLY(...)
#endif

/**
 * Include the wrapped code only on the server or client.
 * 
 * For large sections of code it is better to use `#if ENGINE_SERVER` because Visual
 * Studio (VS only, msvc/cl.exe compiles them fine) is horrible about handling macros and
 * gives lots of false warnings and errors. This is apparently intended and is "not a
 * bug". Very cool.
 *
 * @see https://developercommunity.visualstudio.com/t/msvc-nagging-about-adding-macros-to-hint-file-afte/284851
 * @see https://developercommunity.visualstudio.com/t/cpphint-handling-is-still-broken/487335
 * @see https://developercommunity.visualstudio.com/t/green-squiggles-and-light-bulb-with-macro-in-skipp/248016
 * @see https://learn.microsoft.com/en-us/cpp/build/reference/hint-files?view=msvc-170
 */
#if ENGINE_SERVER
	#define ENGINE_SERVER_ONLY(...) __VA_ARGS__
#else
	#define ENGINE_SERVER_ONLY(...)
#endif

/** @copydoc ENGINE_SERVER_ONLY */
#if ENGINE_CLIENT
	#define ENGINE_CLIENT_ONLY(...) __VA_ARGS__
#else
	#define ENGINE_CLIENT_ONLY(...)
#endif

/**
 * Expands to a comma.
 * Need for passing a comma in macro arguments.
 */
#define ENGINE_COMMA ,

/**
 * Used at the start of an union to prevent implicit conversions.
 * @param sz The size of the enum. Needed for zero initialization.
 */
#define ENGINE_NO_IMPLICIT_CONVERSION(sz) enum class No_Implicit_Conversion : char {} _no_implicit_conversion[sz] = {};

// TODO: look into other inline options
// GCC: https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
// CLANG: https://clang.llvm.org/docs/AttributeReference.html
// MSVC: https://www.reddit.com/r/programming/comments/g9inos/smarter_cc_inlining_with_attribute_flatten/foyoq62/
// MSVC Archive: https://web.archive.org/web/20220317175003/https://old.reddit.com/r/programming/comments/g9inos/smarter_cc_inlining_with_attribute_flatten/foyoq62/
// 
// We have always had __forceinline, and I just added four attributes in this area,
// 
//     [[msvc::forceinline]] - same as __forceinline on a function definition),
//     [[msvc::noinline]] - same as declspec(noinline) on a function,
//     [[msvc::forceinline_calls]] - goes on statements or statement blocks, and attempts to forceinline all calls in that block,
//     [[msvc::noinline_calls]] - don't inline any function calls in this block
// 
// Nothing like flatten (msvc::forceinline_calls isn't recursive, it only does "one level"), although that would be easy to implement
//

/**
 * Attempts to force a function to be inlined.
 * This does not apply the effects of the C++ `inline` keyword.
 */
#define ENGINE_NOINLINE [[msvc::noinline]]
#define ENGINE_INLINE [[msvc::forceinline]] // TODO: cross platform: [[gnu::always_inline]]
#if ENGINE_DEBUG
	//#define ENGINE_INLINE_REL ENGINE_NOINLINE
	#define ENGINE_INLINE_REL
#else
	#define ENGINE_INLINE_REL ENGINE_INLINE
#endif

/**
 * Attempts to force inline all function calls in a block or statement.
 */
#define ENGINE_INLINE_CALLS [[msvc::forceinline_calls]]

/**
 * Attempts to flatten all calls in a block or statement recursively.
 * https://twitter.com/molecularmusing/status/1578657585334714368
 * https://twitter.com/terrym4h/status/1579311539781505026
 * https://web.archive.org/web/20221111110723/https://twitter.com/molecularmusing/status/1578657585334714368
 *
 * MSVC: doesn't work with /Ob1 like forceinline does.
 */
// TODO: once msvc upgrade: #define ENGINE_FLATTEN [[msvc::flatten]] // TODO: cross platform: [[gnu::flatten]]
#define ENGINE_FLATTEN [[msvc::flatten]]

#if ENGINE_OS_WINDOWS
	#define ENGINE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
	#define ENGINE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

/**
 * Disable or restore runtime checks for a region of code.
 * Runtime checks can have an huge cost in compute heavy code. It can be extremely
 * beneficial to _temporarily_ disable them for a specific section. Terrain generation,
 * noise generation, processing large files, etc.
 */
// TODO: MSVC only
#define ENINGE_RUNTIME_CHECKS_DISABLE _Pragma(R"(runtime_checks("", off))")
#define ENINGE_RUNTIME_CHECKS_RESTORE _Pragma(R"(runtime_checks("", restore))")

/**
 * Build various operators for enums.
 */
#define ENGINE_BUILD_BIN_OP(T, O, F) \
	ENGINE_INLINE constexpr F decltype(auto) operator O(const T& a, const T& b) noexcept { \
		return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) O static_cast<std::underlying_type_t<T>>(b)); \
	}

#define ENGINE_BUILD_UNARY_OP(T, O, F) \
	ENGINE_INLINE constexpr F decltype(auto) operator O(const T& a) noexcept { \
		_Pragma("warning(suppress:4146)");\
		return static_cast<T>(O static_cast<std::underlying_type_t<T>>(a)); \
	}

#define ENGINE_BUILD_UNARY_OP_REF(T, O, F) \
	ENGINE_INLINE constexpr F decltype(auto) operator O(T& a) noexcept { \
		auto tmp = static_cast<std::underlying_type_t<T>>(a);\
		return a = static_cast<T>(O tmp);\
	}

#define ENGINE_BUILD_UNARY_OP_INT(T, O, F) \
	ENGINE_INLINE constexpr F decltype(auto) operator O(T& a, int) noexcept { \
		const auto tmp = a;\
		O a;\
		return tmp;\
	}

#define ENGINE_BUILD_ASSIGN_BIN_OP(T, O, F) \
	ENGINE_INLINE constexpr F decltype(auto) operator O=(T& a, const T& b) noexcept { return a = a O b; }

#define ENGINE_BUILD_ALL_OPS_F(T, F) \
	ENGINE_BUILD_UNARY_OP(T, ~, F); \
	ENGINE_BUILD_UNARY_OP(T, !, F); \
	/*ENGINE_BUILD_UNARY_OP(T, +, F); // Excluded because of conflict with ENGINE_BUILD_DECAY_ENUM and doesn't provide anything on its own. */ \
	ENGINE_BUILD_UNARY_OP(T, -, F); \
	ENGINE_BUILD_UNARY_OP_REF(T, ++, F); \
	ENGINE_BUILD_UNARY_OP_REF(T, --, F); \
	ENGINE_BUILD_UNARY_OP_INT(T, ++, F); \
	ENGINE_BUILD_UNARY_OP_INT(T, --, F); \
	ENGINE_BUILD_BIN_OP(T, |, F); \
	ENGINE_BUILD_BIN_OP(T, &, F); \
	ENGINE_BUILD_BIN_OP(T, ^, F); \
	ENGINE_BUILD_BIN_OP(T, +, F); \
	ENGINE_BUILD_BIN_OP(T, -, F); \
	ENGINE_BUILD_BIN_OP(T, *, F); \
	ENGINE_BUILD_BIN_OP(T, /, F); \
	ENGINE_BUILD_BIN_OP(T, %, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, |, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, &, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, ^, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, +, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, -, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, *, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, /, F); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, %, F);

#define ENGINE_BUILD_ALL_OPS(T) ENGINE_BUILD_ALL_OPS_F(T,)

/**
 * Decay an enum to its underlying type.
 */
#define ENGINE_BUILD_DECAY_ENUM(T) \
	constexpr inline decltype(auto) operator+(const T& t) noexcept { return static_cast<std::underlying_type_t<T>>(t); }

/**
 * Used to trigger a static_assert only in a specialized template context.
 * Example: static_assert(ENGINE_TMP_FALSE(T), "Type must be specialized for type T")
 */
#define ENGINE_TMP_FALSE(T) (!sizeof(T))

/**
 * @def ENGINE_EMPTY_BASE
 * Enables more broad use of empty base optimization on MSVC
 * @see https://learn.microsoft.com/en-us/cpp/cpp/empty-bases
 * @see https://devblogs.microsoft.com/cppblog/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
 */
#if ENGINE_OS_WINDOWS
	#define ENGINE_EMPTY_BASE __declspec(empty_bases)
#else
	#define ENGINE_EMPTY_BASE
#endif

/**
 * Insert a breakpoint in debug mode.
 * Useful for unexpected, but tolerable, errors.
 */
#if ENGINE_DEBUG
	#define ENGINE_DEBUG_BREAK __debugbreak();
#else
	#define ENGINE_DEBUG_BREAK
#endif

namespace Engine {
	/**
	 * A generic, unique, constructable, type for none/void/nil/null.
	 */
	struct None { constexpr None() noexcept = default; };
}

/**
 * Helpers for quickly performing ADL.
 */
#define CREATE_ADL_WRAPPER_1(name) \
	template<class T> ENGINE_INLINE constexpr decltype(auto) adl_##name(T&& t) { \
		using ::std::name; return name(std::forward<T>(t)); \
	}
#define CREATE_ADL_WRAPPER_2(name) \
	template<class T> ENGINE_INLINE constexpr decltype(auto) adl_##name(T&& t1, T&& t2) { \
		using ::std::name; return name(std::forward<T>(t1), std::forward<T>(t2)); \
	}
namespace Engine {
	CREATE_ADL_WRAPPER_2(swap);
	CREATE_ADL_WRAPPER_1(data);
	CREATE_ADL_WRAPPER_1(empty);
	CREATE_ADL_WRAPPER_1(size);
	CREATE_ADL_WRAPPER_1(ssize);
	CREATE_ADL_WRAPPER_1(begin);
	CREATE_ADL_WRAPPER_1(cbegin);
	CREATE_ADL_WRAPPER_1(rbegin);
	CREATE_ADL_WRAPPER_1(crbegin);
	CREATE_ADL_WRAPPER_1(end);
	CREATE_ADL_WRAPPER_1(cend);
	CREATE_ADL_WRAPPER_1(rend);
	CREATE_ADL_WRAPPER_1(crend);
}
#undef CREATE_ADL_WRAPPER_1
#undef CREATE_ADL_WRAPPER_2


// Log stuff, must be last due to include order issues between: engine.hpp > detail.hpp > GlobalConfig.hpp > Logger.hpp > engine.hpp
#include <Engine/Types.hpp>
#include <Engine/FatalException.hpp>
#include <Engine/Constants.hpp>
#include <Engine/Detail/detail.hpp>

#if ENGINE_DISABLE_ALL_LOGGING
	#define _ENGINE_CREATE_LOG_LAMBDA(...) ([](auto&&...){})
#else
	#define _ENGINE_CREATE_LOG_LAMBDA(Prefix, Decorate, Color, Other)\
		([](auto&&... args) ENGINE_INLINE {\
			::Engine::Detail::log<Decorate>(Prefix, Color,\
				(__FILE__ + sizeof(ENGINE_BASE_PATH)), __LINE__,\
				std::forward<decltype(args)>(args) ...\
			);\
			Other;\
		})
#endif

#define _ENGINE_CREATE_ASSERT_LAMBDA(Prefix, Decorate, Color, Other)\
	([](auto cond, auto&&... args) ENGINE_INLINE {\
		if (!cond) [[unlikely]] {\
			_ENGINE_CREATE_LOG_LAMBDA(Prefix, Decorate, Color, Other)(std::forward<decltype(args)>(args)...);\
		}\
	})

#if ENGINE_DEBUG
	#define ENGINE_DIE __debugbreak(); ::std::terminate();
#else
	#define ENGINE_DIE ::std::terminate();
#endif

//#define ENGINE_RAW_TEXT _ENGINE_CREATE_LOG_LAMBDA("[TEXT_OLD]", false, Engine::ASCII_FG2, 0)
#define ENGINE_LOG _ENGINE_CREATE_LOG_LAMBDA("[LOG_OLD]", true, Engine::ASCII_FG2, 0)
#define ENGINE_INFO _ENGINE_CREATE_LOG_LAMBDA("[INFO_OLD]", true, Engine::ASCII_INFO, 0)
//#define ENGINE_SUCCESS _ENGINE_CREATE_LOG_LAMBDA("[SUCCESS_OLD]", true, Engine::ASCII_SUCCESS, 0)
#define ENGINE_WARN _ENGINE_CREATE_LOG_LAMBDA("[WARN_OLD]", true, Engine::ASCII_WARN, 0)
//#define ENGINE_FAIL _ENGINE_CREATE_LOG_LAMBDA("[FAIL_OLD]", true, Engine::ASCII_ERROR, 0)
#define ENGINE_ERROR _ENGINE_CREATE_LOG_LAMBDA("[ERROR_OLD]", true, Engine::ASCII_ERROR, ENGINE_DIE)

#define ENGINE_ASSERT _ENGINE_CREATE_ASSERT_LAMBDA("[ERROR][ASSERT]", true, Engine::ASCII_ERROR, ENGINE_DIE)
#define ENGINE_ASSERT_WARN _ENGINE_CREATE_ASSERT_LAMBDA("[WARN][ASSERT]", true, Engine::ASCII_WARN, 0)

#if ENGINE_DEBUG
	#define ENGINE_DEBUG_ASSERT ENGINE_ASSERT
#else
	#define ENGINE_DEBUG_ASSERT(...)
#endif

#if ENGINE_DISABLE_ALL_LOGGING
	#define ENGINE_DEBUG2(...)
	#define ENGINE_LOG2(...)
	#define ENGINE_INFO2(...)
	#define ENGINE_SUCCESS2(...)
	#define ENGINE_VERBOSE2(...)
	#define ENGINE_WARN2(...)
#else
	#define _detail_ENGINE_LOG_USING using ::Engine::Log::Styled; using ::Engine::Log::Style;
	#define ENGINE_DEBUG2(...) { _detail_ENGINE_LOG_USING ::Engine::getGlobalConfig().logger.debug(__VA_ARGS__); }
	#define ENGINE_LOG2(...) { _detail_ENGINE_LOG_USING ::Engine::getGlobalConfig().logger.log(__VA_ARGS__); }
	#define ENGINE_INFO2(...) { _detail_ENGINE_LOG_USING ::Engine::getGlobalConfig().logger.info(__VA_ARGS__); }
	#define ENGINE_SUCCESS2(...) { _detail_ENGINE_LOG_USING ::Engine::getGlobalConfig().logger.success(__VA_ARGS__); }
	#define ENGINE_VERBOSE2(...) { _detail_ENGINE_LOG_USING ::Engine::getGlobalConfig().logger.verbose(__VA_ARGS__); }
	#define ENGINE_WARN2(...) { _detail_ENGINE_LOG_USING ::Engine::getGlobalConfig().logger.warn(__VA_ARGS__); }
#endif

#define ENGINE_ERROR2 ([]<class... Args>(const ::Engine::Log::FormatString<Args...>& format, const Args&... args) ENGINE_INLINE {\
	::Engine::getGlobalConfig().logger.error(format, args...);\
	ENGINE_DIE;\
})

#define ENGINE_CONSOLE ([]<class... Args>(const ::Engine::Log::FormatString<Args...>& format, const Args&... args) ENGINE_INLINE {\
	::Engine::getGlobalConfig().logger.user(format, +::Engine::Log::Level::User, "CONSOLE", ::Engine::Log::Style::FG::Green, args...);\
})


namespace Engine {
	ENGINE_INLINE constexpr char toLower(char c) noexcept {
		return c >= 'A' && c <= 'Z' ? c+32 : c;
	}

	class CompileString {
		public:
			const char* const data;
			consteval CompileString(const char* const data) noexcept : data{data} {}
			constexpr operator std::string_view() const noexcept { return view(); }
			constexpr bool operator==(std::string_view other) const noexcept { return view() == other; };

		private:
			constexpr std::string_view view() const noexcept { return data; }
	};

	ENGINE_INLINE decltype(auto) pbegin(auto& obj) { return std::to_address(std::begin(obj)); }
	ENGINE_INLINE decltype(auto) pcbegin(const auto& obj) { return std::to_address(std::cbegin(obj)); }
	ENGINE_INLINE decltype(auto) pend(auto& obj) { return std::to_address(std::end(obj)); }
	ENGINE_INLINE decltype(auto) pcend(const auto& obj) { return std::to_address(std::cend(obj)); }
}
