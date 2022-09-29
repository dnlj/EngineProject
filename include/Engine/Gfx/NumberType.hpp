#pragma once

// STD
#include <iosfwd>


namespace Engine::Gfx {
	class NumberTypeShape {
		public:
			uint8 cols;
			uint8 rows;
	};

	enum class NumberType {
		#define X(Name, Type, GLEnum, Cols, Rows) Name,
		#include <Engine/Gfx/NumberType.xpp>
		// TODO: half float
		// TODO: fixed point
		// TODO: special values such as GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, etc
		_count,
	};

	ENGINE_BUILD_DECAY_ENUM(NumberType);

	constexpr bool isInteger(NumberType type) noexcept {
		constexpr bool lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum, Cols, Rows) std::integral<Type>,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}
	
	constexpr GLenum toGLEnum(NumberType type) noexcept {
		constexpr GLenum lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum, Cols, Rows) GLEnum,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}
	
	/*
	 * Gets the shape of a NumberType.
	 */
	constexpr NumberTypeShape getShape(NumberType type) noexcept {
		constexpr NumberTypeShape lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum, Cols, Rows) {Cols, Rows},
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}

	/**
	 * Converts from an GLenum to a NumberType.
	 */
	constexpr NumberType fromGLEnum(GLenum type) noexcept {
		switch (type) {
			#define X(Name, Type, GLEnum, Cols, Rows) case GLEnum: { return NumberType::Name; }
			#include <Engine/Gfx/NumberType.xpp>
			default: { return NumberType::Unknown; }
		}
	}

	/**
	 * Gets the size in bytes of the associated type.
	 */
	constexpr uint32 getTypeSize(NumberType type) noexcept {
		constexpr uint32 lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum, Cols, Rows) sizeof(Type),
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}

	constexpr std::string_view toString(NumberType type) noexcept {
		constexpr std::string_view lookup[+NumberType::_count] = {
			#define X(Name, Type, GLEnum, Cols, Rows) "NumberType::"#Name,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}

	template<class T> struct TypeToEnum;
	#define X(Name, TypeM, GLEnum, Cols, Rows) template<> struct TypeToEnum<TypeM> { constexpr static NumberType value = NumberType::Name; };
	#include <Engine/Gfx/NumberType.xpp>
	template<class T> constexpr static inline NumberType TypeToEnum_v = TypeToEnum<T>::value;

	std::ostream& operator<<(std::ostream& os, NumberType type);
}
