#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bit/Bit.hpp>


namespace Engine {
	// TODO: SIMD?
	template<int16 N>
	class Bitset {
		static_assert(N > 0, "Attempting to create empty bitset.");
		public:
			using SizeType = decltype(N);
			using StorageUnit = uint64; // TODO: select best storage type based on N and sys arch;

		private:
			constexpr static SizeType storageUnitBits = sizeof(StorageUnit) * CHAR_BIT;
			constexpr static SizeType storagePow2 = Engine::Bit::ctz(storageUnitBits);
			constexpr static SizeType storageSize = []{
				constexpr auto q = ((N - 1) / storageUnitBits);
				constexpr auto r = ((N - (q * storageUnitBits)) > 0);
				return q + r;
			}();

			StorageUnit storage[storageSize] = {};

		public:
			Bitset() = default;

			// TODO:
			//template<std::integral I>
			//Bitset(I initial) {};

			ENGINE_INLINE bool test(SizeType i) const noexcept { return storage[index(i)] & (1 << bit(i)); };
			ENGINE_INLINE void set(SizeType i) noexcept { storage[index(i)] |= (1 << bit(i)); };
			ENGINE_INLINE void set(SizeType i, bool v) noexcept { reset(i); storage[index(i)] |= (v << bit(i));};
			ENGINE_INLINE void reset(SizeType i) noexcept { storage[index(i)] &= ~(1 << bit(i)); };
			ENGINE_INLINE void flip(SizeType i) noexcept { storage[index(i)] ^= 1 << bit(i); }
			ENGINE_INLINE void invert() noexcept { for (SizeType i = 0; i < storageSize; ++i) { storage[i] = ~storage[i]; } }

			// TODO: [cr]begin / [cr]end

			byte* data() noexcept { return storage; } // TODO: will need some way to get data size in bytes;
			const byte* data() const noexcept { return reinterpret_cast<Bitset*>(this)->data(); }

			constexpr static SizeType size() noexcept { return N; }
			constexpr static SizeType capacity() noexcept { return sizeof(storage) * CHAR_BIT; }

			// TODO: |, &, ~, ^, <<, >> and assignment versions

			// TODO: integral version
			ENGINE_INLINE Bitset& operator&=(const Bitset& other) { for (SizeType i = 0; i < storageSize; ++i) { storage[i] &= other.storage[i]; } }
			ENGINE_INLINE Bitset& operator|=(const Bitset& other) { for (SizeType i = 0; i < storageSize; ++i) { storage[i] |= other.storage[i]; } }
			ENGINE_INLINE Bitset& operator^=(const Bitset& other) { for (SizeType i = 0; i < storageSize; ++i) { storage[i] ^= other.storage[i]; } }
			ENGINE_INLINE Bitset operator~() { Bitset n{*this}; n.invert(); return n; }

			ENGINE_INLINE friend bool operator==(Bitset a, Bitset b) {
				for (SizeType i = 0; i < storageSize; ++i) {
					if (storage[i] != other.storage[i]) {
						return false;
					}
				}
				return true;
			}

			ENGINE_INLINE friend bool operator!=(Bitset a, Bitset b) { return !(a == b); }

		private:
			ENGINE_INLINE constexpr static SizeType index(SizeType i) { return i >> storagePow2; };
			ENGINE_INLINE constexpr static SizeType bit(SizeType i) { return i & (storageUnitBits - 1); };
	};

	// TODO: hash specialization
}
