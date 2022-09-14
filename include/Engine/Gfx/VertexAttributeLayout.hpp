#pragma once

// Engine
#include <Engine/Gfx/NumberType.hpp>
#include <Engine/Gfx/VertexInput.hpp>


namespace Engine::Gfx {
	// Describes the location of a specific attribute within a buffer
	class VertexAttributeDesc {
		public:
			VertexAttributeDesc() = default;
			VertexAttributeDesc(VertexInput input, uint16 size, NumberType type, uint32 offset, bool normalize, uint8 binding, uint32 divisor)
				: input{input}
				, type{type}
				, offset{offset}
				, size{size}
				, normalize{normalize}
				, binding{binding}
				, divisor{divisor} {
			}

			VertexInput input = {};
			NumberType type = {};
			uint32 offset = {};
			uint16 size = {};
			bool normalize = {};
			uint8 binding = {};
			uint32 divisor = {};
			bool operator==(const VertexAttributeDesc&) const noexcept = default;
	}; static_assert(sizeof(VertexAttributeDesc) == 20);

	class VertexAttributeDescList {
		private:
			constexpr static size_t Count = 16;
			VertexAttributeDesc attribs[Count]; // TODO: make VertexAttributeDescList always have sorted attribs

		public:
			template<size_t N>
			VertexAttributeDescList(const VertexAttributeDesc (&init)[N]) {
				static_assert(N <= Count, "To many vertex attributes");
				std::copy(init, init+N, attribs);
			}

			VertexAttributeDescList(const VertexAttributeDesc* data, size_t count) {
				ENGINE_ASSERT_WARN(count <= Count, "To many vertex attributes");
				std::copy(data, data + std::min(count, Count), attribs);
			}

			ENGINE_INLINE constexpr auto size() const { return Count; }
			ENGINE_INLINE const auto begin() const { return attribs; }
			ENGINE_INLINE const auto end() const { return attribs + Count; }
			ENGINE_INLINE const auto cbegin() const { return begin(); }
			ENGINE_INLINE const auto cend() const { return end(); }
			const VertexAttributeDesc& operator[](uint32 i) const { return attribs[i]; }
			bool operator==(const VertexAttributeDescList&) const = default;
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
struct Engine::Hash<Engine::Gfx::VertexAttributeDescList> {
	size_t operator()(const Engine::Gfx::VertexAttributeDescList& val) const {
		return hashBytes(val.begin(), val.size() * sizeof(val[0]));
	}
};
