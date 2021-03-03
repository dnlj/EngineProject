#version 450 core

layout (location = 0) in vec2 vertPosition;
layout (location = 1) uniform mat4 mvp;
out vec2 fragPosition;

void main() {
	fragPosition = vertPosition;
	gl_Position = mvp * vec4(vertPosition, 0.0, 1.0);
}
