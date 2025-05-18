#pragma once

// STD
#include <type_traits>


namespace Engine {
	/**
	 * When inherited from, uses empty base optimization (ebo) to provide a member of type @p T.
	 * The dervied type may need to use #ENGINE_EMPTY_BASE for empty bases to be optimized.
	 * @see ENGINE_EMPTY_BASE
	 */
	template<class T>
	class ENGINE_EMPTY_BASE BaseMember {
		private:
			T value;

		public:
			constexpr BaseMember() {}

			template<class... Args>
			constexpr BaseMember(Args&&... args) : value(std::forward<Args>(args)...) {}

			[[nodiscard]] ENGINE_INLINE constexpr T& get() noexcept { return value; }
			[[nodiscard]] ENGINE_INLINE constexpr const T& get() const noexcept { return value; }
	};

	template<>
	class ENGINE_EMPTY_BASE BaseMember<void> {
		public:
			constexpr BaseMember() {}
			ENGINE_INLINE constexpr void get() const noexcept {}
	};

	/** @see BaseMember */
	template<class T>
	requires std::is_empty_v<T>
	class ENGINE_EMPTY_BASE BaseMember<T> : public T {
		public:
			constexpr BaseMember() {}

			template<class... Args>
			constexpr BaseMember(Args&&... args) : T(std::forward<Args>(args)...) {}

			[[nodiscard]] ENGINE_INLINE constexpr T& get() noexcept { return *this; }
			[[nodiscard]] ENGINE_INLINE constexpr const T& get() const noexcept { return *this; }
	};
}
