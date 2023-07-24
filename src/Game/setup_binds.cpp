// Engine
#include <Engine/Input/BindManager.hpp>
#include <Engine/UI/Context.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/InputSystem.hpp>
#include <Game/systems/ActionSystem.hpp>

void setupBinds(Game::EngineInstance& engine) {
	using namespace Engine::Input;
	using Layer = Game::InputLayer;
	using Action = Game::Action;
	using Type = InputType;

	auto& world = engine.getWorld();
	auto& guiContext = engine.getUIContext();
	auto& bm = engine.getBindManager();
	auto& is = world.getSystem<Game::InputSystem>();

	constexpr auto updateActionState = [](auto& world, auto action, auto curr) ENGINE_INLINE {
		if constexpr (ENGINE_SERVER) { return; }
		world.getSystem<Game::ActionSystem>().updateActionState(action, curr.i32);
	};
	constexpr auto updateTargetState = [](auto& world, auto curr) ENGINE_INLINE {
		if constexpr (ENGINE_SERVER) { return; }
		world.getSystem<Game::ActionSystem>().updateTarget(curr.f32v2);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Game Binds
	////////////////////////////////////////////////////////////////////////////////////////////////
	is.registerCommand(Action::Attack1, [&](Value curr){ updateActionState(world, Action::Attack1, curr); });
	is.registerCommand(Action::Attack2, [&](Value curr){ updateActionState(world, Action::Attack2, curr); });
	is.registerCommand(Action::MoveUp, [&](Value curr){ updateActionState(world, Action::MoveUp, curr); });
	is.registerCommand(Action::MoveDown, [&](Value curr){ updateActionState(world, Action::MoveDown, curr); });
	is.registerCommand(Action::MoveLeft, [&](Value curr){ updateActionState(world, Action::MoveLeft, curr); });
	is.registerCommand(Action::MoveRight, [&](Value curr){ updateActionState(world, Action::MoveRight, curr); });
	is.registerCommand(Action::Target, [&](Value curr){ updateTargetState(world, curr); });

	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 29}, // CTRL
		InputId{Type::Keyboard, 0, 46}, // C
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::Attack1, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 29}, // CTRL
		InputId{Type::Keyboard, 0, 56}, // ALT
		InputId{Type::Keyboard, 0, 16}, // Q
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::Attack1, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 57}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::Attack1, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 17}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::MoveUp, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 31}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::MoveDown, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 30}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::MoveLeft, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Keyboard, 0, 32}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::MoveRight, time, curr); return true; });

	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Mouse, 0, 0}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::Attack1, time, curr); return true; });
	bm.addBind(Layer::Game, false, InputSequence{
		InputId{Type::Mouse, 0, 1}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::Attack2, time, curr); return true; });

	bm.addBind(Layer::Game, false, InputSequence{
			InputId{Type::MouseAxis, 0, 0}
	}, [&](Value curr, Value prev, auto time){ is.pushEvent(Action::Target, time, curr); return true; });

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Interface Binds
	////////////////////////////////////////////////////////////////////////////////////////////////
	using GuiAction = Engine::UI::Action;

	// Chars
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Left},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveCharLeft); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Right},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveCharRight); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Up},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveCharUp); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Down},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveCharDown); } return true; });


	// Words
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::Left},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveWordLeft); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::Left},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveWordLeft); } return true; });

	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::Right},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveWordRight); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::Right},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveWordRight); } return true; });


	// Lines
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Home},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveLineStart); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::End},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::MoveLineEnd); } return true; });

	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Backspace},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::DeletePrev); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Delete},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::DeleteNext); } return true; });


	// Selection
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LShift},
		}, [&](Value curr, Value prev, auto time){
		if (curr != prev) {
			guiContext.queueFocusAction(curr.i32 ? GuiAction::SelectBegin : GuiAction::SelectEnd);
		}
		return true;
	});
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RShift},
	}, [&](Value curr, Value prev, auto time){
		if (curr != prev) {
			guiContext.queueFocusAction(curr.i32 ? GuiAction::SelectBegin : GuiAction::SelectEnd);
		}
		return true;
	});

	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::A},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::SelectAll); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::A},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::SelectAll); } return true; });

	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Enter},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Submit); } return true; });


	// Cut
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::X},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Cut); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::X},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Cut); } return true; });


	// Copy
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::C},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Copy); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::C},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Copy); } return true; });


	// Paste
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::V},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Paste); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RCtrl},
		InputId{Type::Keyboard, 0, +KeyCode::V},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::Paste); } return true; });


	// Scroll
	bm.addBind(Layer::GuiHover, true, InputSequence{
			InputId{Type::MouseWheel, 0, 0}
	}, [&](Value curr, Value prev, auto time){ guiContext.queueHoverAction(GuiAction::Scroll, curr); return true; });


	bm.addBind(Layer::GuiHover, true, InputSequence{
			InputId{Type::Mouse, 0, 0}
	}, [&](Value curr, Value prev, auto time){
		if (curr.i32) { guiContext.focusHover(); }
		return guiContext.onActivate(curr.i32, time);
	});


	// TODO: really want a way to specify generic L/R shift without to registers
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::Tab},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::PanelNext); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::LShift},
		InputId{Type::Keyboard, 0, +KeyCode::Tab},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::PanelPrev); } return true; });
	bm.addBind(Layer::GuiFocus, true, InputSequence{
		InputId{Type::Keyboard, 0, +KeyCode::RShift},
		InputId{Type::Keyboard, 0, +KeyCode::Tab},
	}, [&](Value curr, Value prev, auto time){ if (curr.i32) { guiContext.queueFocusAction(GuiAction::PanelPrev); } return true; });
	
	//bm.setLayerEnabled(Layer::GuiFocus, false);
}
