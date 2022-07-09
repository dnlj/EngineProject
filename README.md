# Engine Project
TODO: project desc

## Entity Component System
An Entity-Component-System (ECS) architecture is a way of structuring your program around the ideas
of Entities, Components, and Systems. Entities are simply identifiers that are used to associate
groups of data. Components are the pieces of data being grouped. And Systems are the logic/behaviors,
and will typically operate on all entities that have a certain set of components.

### Example
Here's a basic example of what usage might look like.

```C++
struct PhysicsComponent {
	vec2 position = {0, 0};
	vec2 velocity = {0, 0};
	vec2 acceleration = {0, 0};
};

struct SpriteComponent {
	SpriteRef sprite;
};

struct PhysicsSystem : System {
	using System::System;
	void tick() { // Run at a fixed update interval specified by the world
		constexpr auto dt = world.getTickDelta();
		for (Entity ent : world.getFilter<PhysicsComponent>()) {
			auto& phys = world.getComponent<PhysicsComponent>(ent);
			phys.acceleration += dt * gravity;
			phys.velocity += dt * phys.acceleration;
			phys.position += dt * phys.velocity;
		}
	}
};

struct RenderSystem : System {
	using System::System;
	void update(float32 dt) { // Run every frame
		for (Entity ent : world.getFilter<SpriteComponent, PhysicsComponent>()) {
			const auto& [spriteComp, physComp] = world.getComponents<SpriteComponent, PhysicsComponent>(ent);
			// Really we should interpolate between current position and previous
			// position with dt to get smooth movement at any framerate.
			renderSpriteAtPosition(spriteComp.sprite, physComp.position);
		}
	}
}

World world;
Entity ent = world.createEntity();
world.addComponent<PhysicsComponent>(ent);
world.addComponent<SpriteComponent>(ent, sprite);

while (!window.shouldClose()) {
	window.poll();
	world.run();
	window.swapBuffers();
}

```

## Networking
Networking is implemented on top of UDP using messages and channels. A message is a group of data that is sent over a specific channel. A channel is a logical sequence of related messages. Channels can provide various guarantees and functionality for messages such as reliability, ordering, priority, and large message splitting which can all be configured and a per channel basis.

In the background messages from multiple channels are combined into a UDP packet, sent, and reassembled on the receiving side.

### Example
Here is an example of what usage might look like. This API is likely to change in the future as im not really happy with how bare bones and error prone this can be.
```C++
void send(Connection& conn) {
	float32 foo = 3.14159f;
	std::string bar = "The quick brown fox jumps over the lazy dog.";

	if (auto msg = conn.beginMessage<Messages::MyExampleMessage>()) {
		msg.write(foo);
		msg.write(bar.size());
		msg.write(bar.data(), bar.size());
	}
}

void recv(Connection& conn) {
	if (auto* foo = conn.read<float32>()) {
		*foo;
		// ...
	}

	if (auto* len = conn.read<uint64>()) {
		char* str = conn.read(*len);
		std::string(str, *len);
		// ...
	}
}
```

## User Interface
The user interface library uses OpenGL for rendering, FreeType for glyph generation, and HarfBuzz for layout.

With the current rendering implementation there is a lot of room for improvement with how draw batches are handled. Specifically around clipping, textures, and text rendering. In the future it should be possible to reduce the number of draw calls by a huge margin.

Currently there is no support for fallbacks or full color emoji, these features will likely be added in the future.

### Example
Here is an example of a creating a window with a text box and button.

```C++
auto win = ctx.createPanel<Window>(ctx.getRoot());
win->setTitle("UI Example");
win->setSize({275, 100});

auto cont = win->getContent();
cont->setLayout(new DirectionalLayout{Direction::Horizontal, Align::Start, Align::Start, theme.sizes.pad1});

auto text = ctx.createPanel<TextBox>(cont);
text->autoText("This is a editable text box!");

auto btn = ctx.createPanel<Button>(cont);
btn->autoText("Submit");
```
![Simple user interface example window](/docs/img/ui_simple_example.png)

Here is a more complex example including graphs, clipping, scroll sections, editable text, sliders, collapsible sections, etc.

![Complex user interface example window](/docs/img/ui_complex_example.png)

## Rendering
Currently being heavily reworked. Check back later.

## Utilities and Data Structures
- TODO: data structures (SparseSet, StaticVector, Bitset, RingBuffer(static/dynamic), SeqBuffer, etc)
- TODO: utilities
- TODO: command line
- TODO: file type (see example.cfg)

# Build
- TODO: link to docs/building.md
- TODO: Conan stuff
- TODO: Premake stuff

# Game
- TODO: game level networking (interp, buffer, etc)
- TODO: terrain gen
![Terrain physics bounding boxes](/docs/img/terrain_physics_debug.png)
![Terrain with textured wireframe](/docs/img/terrain_textured_wireframe.png)
