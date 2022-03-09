#version 450 core

// TODO: any benefit to packing these with `component` specifier?
layout (location = 0) in vec4 vertColor;
layout (location = 1) in vec2 vertTexCoord;
layout (location = 2) in vec2 vertPos;

layout (location = 0) uniform vec2 viewScale; // = 2 / viewSize

out vec4 fragColor;
out vec2 fragTexCoord;

void main() {
	// Convert from pixels to NDC
	gl_Position = vec4(vertPos * viewScale - 1, 0, 1);
	gl_Position.y = -gl_Position.y;

	fragColor = vertColor;
	fragTexCoord = vertTexCoord;
}
