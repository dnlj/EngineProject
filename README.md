# Engine Project
A game engine I have been working on in my free time. Very much a work in progress.

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
Networking is implemented on top of UDP using messages and channels. A message is a group of data that is sent over a specific channel. A channel is a logical sequence of related messages. Channels can provide various guarantees and functionality for messages such as reliability, ordering, priority, and large message splitting which can all be configured on a per channel basis.

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
The user interface library uses OpenGL for rendering, FreeType for glyph generation, and HarfBuzz for text layout.

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

### Data Structures
- StaticVector: An array/vector like data structure with a static capacity and dynamic size.
- RingBuffer: A typical ring buffer with variants for both static and dynamic capacity.
- Bitset: A bitset similar to `std::bitset` but allows access to the underlying data. (serialization, networking, etc.)
- SparseSet: TODO: description
- SequenceBuffer: TODO: description

### Command Line Parser
A utility class for reading, interpreting, and defaulting command line arguments.

#### Example
```C++
// Setup: add<Type>(full name, short, default, description)
parser
	.add<uint16>("port", 'p', 21212,
		"The port to listen on.")
	.add<IPv4Address>("group", 'g', {224,0,0,212, 12121},
		"The multicast group to join for server discovery. Zero to disable.")
	.add<std::string>("log", 'l', "",
		"The file to use for logging.")
	.add<bool>("logColor",
		"Enable or disable color log output.")
	.add<bool>("logTimeOnly",
		"Show only time stamps instead of full dates in log output.");

// Reading
auto* port = parser.get<uint16>("port");
if (port) { ... }
```

### Config File Parser
An `.ini` inspired format with support for data types, multiline strings, and trailing comments.
One of the main benefit of this format is the ability to maintain source formatting such as whitespace and comments when edited programmatically.

#### Example
A file might look something like this:
```
# example.cfg
[NumberExamples]
bool_var = true # Trailing comments
bin_var = 0b101010
hex_var = 0xDEADBEEF
dec_var = 12345
float_var = -123.67e+10

[AnotherSection]
string_key = "This is a
multiline string example \"with a quote\""
```

Using the parser:
```C++
// Load from file
parser.loadAndTokenize("example.cfg");

// Modify
parser.insert("AnotherSection.new_key", 1.618);
parser.remove("NumberExamples.bin_var");
parser.save("example_modified.cfg");

// Reading
auto* dec_var = parser.get<int64>("NumberExamples.dec_var");
if (dec_var) { ... }
```

# Build
[See building.md](building.md)

# Game
- TODO: game level networking (interp, buffer, etc)

### Terrain Generation
- TODO: terrain gen (2d, tile based, noise functions, etc)

Physics bounds generation:
![Terrain physics bounding boxes](/docs/img/terrain_physics_debug.png)

Rendering bounds generation:
![Terrain with textured wireframe](/docs/img/terrain_textured_wireframe.png)



