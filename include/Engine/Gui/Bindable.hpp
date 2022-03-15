#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>


namespace Engine::Gui {
	template<class Self>
	class Bindable {
		public:
			using GetFunc = std::function<void(Self&)>;
			using SetFunc = std::function<void(Self&)>;

		private:
			SetFunc setFunc;
			ENGINE_INLINE Self& self() noexcept { return static_cast<Self&>(*this); }

		public:
			ENGINE_INLINE void setBindableValue() { if (setFunc) { setFunc(self());} }

			Self& bind(GetFunc get, SetFunc set) {
				static_assert(std::is_base_of_v<Panel, Self>, "Bindable classes must be panels to use addPanelUpdateFunc");
				setFunc = {};
				if (get) {
					get(self());
					self().getContext()->addPanelUpdateFunc(&self(), [g=std::move(get)](Panel* p) {
						g(static_cast<Self&>(*p));
					});
				}
				setFunc = std::move(set);
				return self();
			}
	};
}
