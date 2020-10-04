#pragma once

// STD
#include <iostream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bit/Bit.hpp>
#include <Engine/Hash.hpp>


namespace Engine {
	// TODO: SIMD?
	template<int16 N, class StorageUnit = uint64> // TODO: select best storage type based on N and sys arch;
	class Bitset {
		static_assert(N > 0, "Attempting to create empty bitset.");
		public:
			using SizeType = decltype(N);

			template<int16 M, class U>
			friend class Bitset;

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

			template<std::integral I>
			Bitset(I initial) {
				if constexpr (sizeof(I) > sizeof(StorageUnit)) {
					// TODO: handle
					static_assert(false, "TODO: impl");
				} else {
					storage[0] = initial;
				}
			};

			template<auto M, class U>
			Bitset(const Bitset<M, U>& other) {
				memcpy(storage, other.storage, std::min(sizeof(storage), sizeof(other.storage)));
			};

			// TODO: ref(SizeType i);

			ENGINE_INLINE bool test(SizeType i) const noexcept { return storage[index(i)] & (StorageUnit{1} << bit(i)); };

			ENGINE_INLINE void set() noexcept { /* TODO: impl */ };
			ENGINE_INLINE void set(SizeType i) noexcept { storage[index(i)] |= (StorageUnit{1} << bit(i)); };
			ENGINE_INLINE void set(SizeType i, bool v) noexcept { reset(i); storage[index(i)] |= (v << bit(i));};

			ENGINE_INLINE void reset() noexcept { for (SizeType i = 0; i < storageSize; ++i) { storage[i] = 0; } };
			ENGINE_INLINE void reset(SizeType i) noexcept { storage[index(i)] &= ~(StorageUnit{1} << bit(i)); };

			ENGINE_INLINE void flip(SizeType i) noexcept { storage[index(i)] ^= StorageUnit{1} << bit(i); }

			ENGINE_INLINE void invert() noexcept { for (SizeType i = 0; i < storageSize; ++i) { storage[i] = ~storage[i]; } }

			// TODO: [cr]begin / [cr]end

			ENGINE_INLINE byte* data() noexcept { return reinterpret_cast<byte*>(storage); }
			ENGINE_INLINE const byte* data() const noexcept { return const_cast<Bitset*>(this)->data(); }
			constexpr static int32 dataSize() { return sizeof(storage); }

			constexpr static SizeType size() noexcept { return N; }
			constexpr static SizeType capacity() noexcept { return sizeof(storage) * CHAR_BIT; }

			ENGINE_INLINE Bitset& operator&=(const Bitset& other) noexcept { for (SizeType i = 0; i < storageSize; ++i) { storage[i] &= other.storage[i]; } return *this; }
			ENGINE_INLINE Bitset operator&(const Bitset& other) const noexcept {auto n = *this; return n &= other; }

			ENGINE_INLINE Bitset& operator|=(const Bitset& other) noexcept { for (SizeType i = 0; i < storageSize; ++i) { storage[i] |= other.storage[i]; } return *this; }
			ENGINE_INLINE Bitset operator|(const Bitset& other) const noexcept {auto n = *this; return n |= other; }

			ENGINE_INLINE Bitset& operator^=(const Bitset& other) noexcept { for (SizeType i = 0; i < storageSize; ++i) { storage[i] ^= other.storage[i]; } return *this; }
			ENGINE_INLINE Bitset operator^(const Bitset& other) const noexcept {auto n = *this; return n ^= other; }

			ENGINE_INLINE Bitset operator~() const noexcept { Bitset n{*this}; n.invert(); return n; }

			ENGINE_INLINE operator bool() const { for (SizeType i = 0; i < storageSize; ++i) { if (storage[i]) { return true; } } return false; }

			template<std::integral I>
			Bitset& operator>>=(I n) noexcept {
				while (n > 0) {
					const auto b = std::min<I>(n, storageUnitBits - 1);
					const auto carryBits = storageUnitBits - b;

					for (SizeType i = 0; i < storageSize - 1; ++i) {
						auto& s = storage[i];
						s >>= b;
						s |= storage[i + 1] << carryBits;
					}

					storage[storageSize - 1] >>= b;
					n -= b;
				}
				return *this;
			}
			
			template<std::integral I>
			Bitset& operator<<=(I n) noexcept {
				while (n > 0) {
					const auto b = std::min<I>(n, storageUnitBits - 1);
					const auto carryBits = storageUnitBits - b;

					for (SizeType i = storageSize - 1; i > 0; --i) {
						auto& s = storage[i];
						s <<= b;
						s |= storage[i - 1] >> carryBits;
					}

					storage[0] <<= b;
					n -= b;
				}
				return *this;
			}
			
			template<std::integral I>
			ENGINE_INLINE Bitset operator>>(I n) const noexcept { auto copy = *this; return copy >>= n; }

			template<std::integral I>
			ENGINE_INLINE Bitset operator<<(I n) const noexcept { auto copy = *this; return copy <<= n; }

			ENGINE_INLINE friend bool operator==(const Bitset& a, const Bitset& b) noexcept {
				for (SizeType i = 0; i < storageSize; ++i) {
					if (a.storage[i] != b.storage[i]) {
						return false;
					}
				}
				return true;
			}

			ENGINE_INLINE friend bool operator!=(const Bitset& a, const Bitset& b) noexcept { return !(a == b); }

			friend std::ostream& operator<<(std::ostream& os, const Bitset& bs) {
				for (SizeType i = storageSize - 1; i >= 0; --i) {
					for (size_t bit = 0; bit < storageUnitBits; ++bit) {
						os << ((bs.storage[i] >> (storageUnitBits - bit - 1)) & 1);
					}
				}
				return os;
			}


		private:
			ENGINE_INLINE constexpr static SizeType index(SizeType i) noexcept { return i >> storagePow2; };
			ENGINE_INLINE constexpr static SizeType bit(SizeType i) noexcept { return i & (storageUnitBits - 1); };
	};
	

	template<auto I, class T> struct Hash<Bitset<I, T>> {
		uint64 operator()(const Bitset<I, T>& v) const noexcept {
			uint64 seed = 0;
			using st = Bitset<I, T>::SizeType;

			if constexpr (v.dataSize() % 8 == 0) {
				for (st i = 0; i < v.dataSize(); i += 8) {
					hashCombine(seed, reinterpret_cast<const uint64&>(v.data()[i]));
				}
				return seed;
			} else if constexpr (v.dataSize() % 4 == 0) {
				for (st i = 0; i < v.dataSize(); i += 4) {
					hashCombine(seed, reinterpret_cast<const uint32&>(v.data()[i]));
				}
				return seed;
			} else {
				for (Bitset<I, T>::SizeType i = 0; i < v.dataSize(); ++i) {
					hashCombine(seed, v.data()[i]);
				}
				return seed;
			}
		}
	};
}
