#pragma once

// Engine
#include <Engine/Gfx/NumberType.hpp>


namespace Engine::Gfx {
	enum class VertexAttribTarget : uint8 {
		Float,
		Int,
	};

	// Describes the location of a specific attribute within a buffer
	class VertexAttributeDesc {
		public:
			uint16 input = {};
			uint16 size = {};
			NumberType type = {};
			VertexAttribTarget target = {};
			bool normalize = {};
			uint16 offset = {};
			uint8 binding = {};
			uint32 divisor = {};
			bool operator==(const VertexAttributeDesc&) const noexcept = default;
	}; static_assert(sizeof(VertexAttributeDesc) == 20);

	class VertexAttributeLayoutDesc {
		public:
			struct BindingDivisor {
				int32 binding; uint32 divisor;
				bool operator==(const BindingDivisor&) const noexcept = default;
			};

			std::vector<BindingDivisor> divisors;
			std::vector<VertexAttributeDesc> attribs;
			bool operator==(const VertexAttributeLayoutDesc&) const noexcept = default;
	};

	// Maps a buffer layout to a vertex input shape (built from a VertexAttributeDesc[])
	class VertexAttributeLayout {
		private:
			uint32 vao;

		public:
			class AllowConstruct_Tag {
				friend class VertexAttributeLayout;
				private: constexpr AllowConstruct_Tag() noexcept {};
			};
			constexpr static AllowConstruct_Tag AllowConstruct;

			ENGINE_INLINE explicit VertexAttributeLayout(AllowConstruct_Tag, uint32 vao) noexcept : vao{vao} {}
			ENGINE_INLINE uint32 get() const noexcept{ return vao; }
	};
}

template<>
struct Engine::Hash<Engine::Gfx::VertexAttributeLayoutDesc> {
	size_t operator()(const Engine::Gfx::VertexAttributeLayoutDesc& val) const {
		auto res = hashBytes(val.attribs.data(), val.attribs.size() * sizeof(val.attribs[0]));
		hashCombine(res, hashBytes(val.divisors.data(), val.divisors.size() * sizeof(val.divisors[0])));
		return res;
	}
};
