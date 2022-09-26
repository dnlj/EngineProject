#pragma once

// STD
#include <ranges>

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/Gfx/TextureHandle.hpp>
#include <Engine/Gfx/Material.hpp>
#include <Engine/Gfx/TextureManager.hpp>


namespace Engine::Gfx {
	// TODO: relatively untested, at the time of writing this we dont have any
	// ^^^^: models and scenes complex enough to really thrash this
	
	// TODO: in the end this is really just a LRU cache, probably would be
	// ^^^^: useful to extract into more generic class.

	template<uint8 MaxActiveTextures>
	class ActiveTextureCache {
		private:
			using NodeId = uint8;
			constexpr static NodeId invalid = -1;

			class Node {
				friend ActiveTextureCache;
				private:
					// Can do this with a singly lined list by pointing to parent instead, this is just makes things easier to understand.
					NodeId prev = invalid;
					NodeId next = invalid;

				public:
					TextureHandleGeneric tex;
			};

			FlatHashMap<TextureHandleGeneric, NodeId> texToSlot;
			std::array<Node, MaxActiveTextures> nodes;

			NodeId head = invalid;
			NodeId tail = invalid;
			NodeId unused = invalid;

		public:
			ActiveTextureCache(){
				texToSlot.reserve(MaxActiveTextures);
				clear();
			}

			void use(const MaterialInstance& mat) {
				// I would like to just use `std::views::values` here, but we can not because we
				// are not allowed to specialize `std::get` for our custom type.
				struct Adapter {
					using Texs = std::decay_t<decltype(mat.getTextures())>;
					using ItUnder = Texs::const_iterator;
					const Texs& texs;

					struct It : ItUnder {
						It(ItUnder u) : ItUnder(u) {}
						using ItUnder::ItUnder;
						ENGINE_INLINE TextureHandleGeneric operator*() const { return operator->()->second->tex; }
					};

					ENGINE_INLINE It begin() const { return texs.begin(); }
					ENGINE_INLINE It end() const { return texs.end(); }
				};

				use(Adapter{mat.getTextures()});
			}

			template<class View>
			void use(const View& view) {
				auto end = texToSlot.end();

				// Move any existing textures to front
				for (const TextureHandleGeneric& tex : view) {
					auto found = texToSlot.find(tex);
					if (found != end) {
						moveToFront(found->second);
					}
				}

				// Add any new textures
				for (const TextureHandleGeneric& tex : view) {
					auto found = texToSlot.find(tex);

					if (found == end) {
						NodeId id = invalid;
						if (unused != invalid) {
							// Take from unused list
							id = push(pop(unused));
						} else {
							// Steal from tail
							texToSlot.erase(nodes[tail].tex);
							id = push(pop(tail));
						}

						found = texToSlot.emplace(tex, id).first;
						nodes[id].tex = tex;
						end = texToSlot.end();
					}
				}
			}

			ENGINE_INLINE NodeId get(TextureHandleGeneric tex) {
				const auto found = texToSlot.find(tex);
				return found == texToSlot.end() ? 0 : found->second;
			}

			void clear() {
				texToSlot.clear();

				unused = 0;
				for (NodeId n = 0; n < MaxActiveTextures; ++n) {
					nodes[n].prev = n - 1;
					nodes[n].next = n + 1;
				}
				nodes[0].prev = invalid;
				nodes[MaxActiveTextures - 1].next = invalid;
			}

			ENGINE_INLINE auto begin() const { return nodes.cbegin(); }
			ENGINE_INLINE auto cbegin() const { return begin(); }

			ENGINE_INLINE auto end() const { return nodes.cbegin(); }
			ENGINE_INLINE auto cend() const { return end(); }

		private:
			/**
			 * Remove a node from the tail of a list.
			 */
			ENGINE_INLINE NodeId pop(NodeId& tail) {
				const auto tinit = tail;
				auto& node = nodes[tail];
				tail = node.next;
				node.next = invalid;
				if (tail != invalid) { nodes[tail].prev = invalid; }
				return tinit;
			}

			/**
			 * Remove a node from a list.
			 */
			ENGINE_INLINE NodeId orphan(NodeId i) {
				auto& node = nodes[i];
				if (node.prev != invalid) { nodes[node.prev].next = node.next; }
				if (node.next != invalid) { nodes[node.next].prev = node.prev; }
				node.next = invalid;
				node.prev = invalid;
				return i;
			}

			/**
			 * Remove a node from the active list.
			 */
			ENGINE_INLINE void pull(NodeId i) noexcept {
				auto& node = nodes[i];
				if (head == i) { head = node.prev; }
				if (tail == i) { tail = node.next; }
				orphan(i);
			}

			/**
			 * Add a node to the active list.
			 */
			ENGINE_INLINE NodeId push(NodeId i) noexcept {
				nodes[i].prev = head;
				if (head != invalid) { nodes[head].next = i; }
				if (tail == invalid) { tail = i; }
				return head = i;
			}

			/**
			 * Move a node to the front of the active list.
			 */
			void moveToFront(NodeId i) noexcept {
				pull(i);
				push(i);
			}
	};
}
