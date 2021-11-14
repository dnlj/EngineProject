#pragma once


namespace Engine::Gui {
	/**
	 * Provides a Context::PanelUpdateFunc to manage panels derived from external data using checksums an polling.
	 * 
	 * Uses CRTP.
	 * Derived classes must implement:
	 * - `It`, Some iterator like type used to iterate data items.
	 * - `It begin()`, Gets the begin iterator.
	 * - `It end()`, Gets the end iterator.
	 * - `Id getId(It it)`, Gets the id for an item.
	 * - `Checksum check(Id id)`, Gets the checksum for an item. Used to determine if panels need updating.
	 * - `Panel* createPanel(Id id, Context& ctx)`, Creates a panel for an item.
	 * - `void updatePanel(Id id, Panel* panel)`, Optional. Updates an existing panel for an item.
	 * - `bool filter(Id id)`, Optional. Determines if an item should be used.
	 *
	 * @tparam Self_ The CRTP derived class.
	 * @tparam Id_ The type used to identify an item.
	 * @tparam Checksum_ The checksum type used to determine if an item has changed.
	 * 
	 */
	template<class Self_, class Id_, class Checksum_>
	class DataAdapter {
		public:
			using Self = Self_;
			using Id = Id_;
			using Checksum = Checksum_;

		private:
			struct Store {
				Panel* panel = nullptr;
				Checksum checksum = 0;
				uint32 iter = 0;
			};

			Engine::FlatHashMap<Id, Store> cache;
			uint32 iter = 0;

		public:
			ENGINE_INLINE Self& self() noexcept { return *reinterpret_cast<Self*>(this); }
			ENGINE_INLINE bool filter(const Id&) const noexcept { return true; }
			ENGINE_INLINE void updatePanel(Id id, Panel* panel) const noexcept {}

			void operator()(Panel* parent) {
				++iter;

				// Create and update items
				for (auto it = self().begin(), e = self().end(); it != e; ++it) {
					auto id = self().getId(it);
					if (!self().filter(id)) { continue; }
					auto found = cache.find(id);
					if (found == cache.end()) {
						auto [f2, _] = cache.emplace(id, Store{
							.panel = self().createPanel(id, *parent->getContext()),
							.checksum = self().check(id),
							.iter = iter,
						});
						found = f2;
						ENGINE_LOG("Add child ", parent, " ", found->second.panel);
						parent->addChild(found->second.panel);
					} else {
						found->second.iter = iter;
						if (auto sum = self().check(id); sum != found->second.checksum) {
							self().updatePanel(id, found->second.panel);
							found->second.checksum = sum;
						}
					}
				}

				// Remove old items
				for (auto it = cache.begin(), e = cache.end(); it != e;) {
					if (it->second.iter != iter) {
						parent->getContext()->deletePanel(it->second.panel);
						it = cache.erase(it);
					} else {
						++it;
					}
				}
			}
	};
}
