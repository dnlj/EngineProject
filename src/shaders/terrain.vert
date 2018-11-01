#version 450 core

layout (location = 0) in vec2 vertPosition;

out vec2 fragPosition;

layout (location = 1) uniform mat4 mvp;

void main() {
	fragPosition = vertPosition;
	gl_Position = mvp * vec4(vertPosition, 0.0, 1.0);
}
