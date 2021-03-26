#version 450 core

layout (location = 0) in vec2 vertPosition;
layout (location = 1) in float vertTexLayer;

out vec2 fragPosition;
out float fragTexLayer;

layout (location = 0) uniform mat4 mvp;

void main() {
	fragPosition = vertPosition;
	fragTexLayer = vertTexLayer;
	gl_Position = mvp * vec4(vertPosition, 0.0, 1.0);
}
