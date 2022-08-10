#pragma once

// STD
#include <iosfwd>


namespace Engine::Gfx {
	enum class NumberType {
		#define X(Name, Type, GLEnum) Name,
		#include <Engine/Gfx/NumberType.xpp>
		// TODO: half float
		// TODO: fixed point
		// TODO: special values such as GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, etc
		_count,
	};

	ENGINE_BUILD_DECAY_ENUM(NumberType);

	constexpr inline bool isInteger(NumberType type) noexcept {
		constexpr bool lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum) std::integral<Type>,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}
	
	constexpr inline GLenum toGLEnum(NumberType type) noexcept {
		constexpr GLenum lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum) GLEnum,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}

	/**
	 * Converts from an GLenum to a NumberType.
	 */
	constexpr inline NumberType fromGLEnum(GLenum type) noexcept {
		switch (type) {
			#define X(Name, Type, GLEnum) case GLEnum: { return NumberType::Name; }
			#include <Engine/Gfx/NumberType.xpp>
			default: { return NumberType::Unknown; }
		}
	}

	/**
	 * Gets the size in bytes of the associated type.
	 */
	constexpr inline uint32 getTypeSize(NumberType type) noexcept {
		constexpr uint32 lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum) sizeof(Type),
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}

	constexpr inline std::string_view toString(NumberType type) noexcept {
		constexpr std::string_view lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum) "NumberType::"#Name,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}

	template<class T> struct TypeToEnum;
	#define X(Name, TypeM, GLEnum) template<> struct TypeToEnum<TypeM> { constexpr static NumberType value = NumberType::Name; };
	#include <Engine/Gfx/NumberType.xpp>
	template<class T> constexpr static inline NumberType TypeToEnum_v = TypeToEnum<T>::value;

	std::ostream& operator<<(std::ostream& os, NumberType type);
}
