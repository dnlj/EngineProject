#version 450 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec2 vertTexCoord;

layout (location = 0) uniform vec2 viewSize;

out vec2 fragTexCoord;

void main() {
	// Convert from pixels to NDC
	gl_Position = vec4((vertPos / viewSize) * 2 - 1, 0, 1);
	gl_Position.y = -gl_Position.y;

	fragTexCoord = vertTexCoord;
}
