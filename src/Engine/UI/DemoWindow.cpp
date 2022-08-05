// Engine
#include <Engine/UI/DemoWindow.hpp>
#include <Engine/UI/Context.hpp>
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/CollapsibleSection.hpp>
#include <Engine/UI/Slider.hpp>
#include <Engine/UI/ImageDisplay.hpp>


namespace Engine::UI {
	DemoWindow::DemoWindow(Context* context) : Window{context} {
		auto& theme = ctx->getTheme();
		auto cont = getContent();
		cont->setLayout(new DirectionalLayout{Direction::Vertical, Align::Start, Align::Stretch, theme.sizes.pad1});

		auto section = [&](auto title) {
			auto sec = ctx->createPanel<CollapsibleSection>(cont);
			auto scon = sec->getContent();
			scon->setLayout(new DirectionalLayout{Direction::Vertical, Align::Start, Align::Start, theme.sizes.pad1, 0});
			sec->setTitle(title);
			return scon;
		};

		{
			auto sec = section("Basic Controls");

			auto label = ctx->createPanel<Label>(sec);
			label->autoText("This is a label");
			
			auto btn = ctx->createPanel<Button>(sec);
			btn->autoText("This is a button: 0");
			btn->setAction([count = 0](auto btn) mutable {
				btn->autoText("This is a button: " + std::to_string(++count));
			});

			auto slider = ctx->createPanel<Slider>(sec);
			slider->setWidth(256);
		}

		{
			auto sec = section("Graphics");
			auto img = ctx->createPanel<ImageDisplay>(sec);
			Gfx::Image store = "nope";
			tex.setAuto(store);
			img->setTexture(tex);
			img->setSize({256,256});

			// TODO: graph demo
		}
	}
}
