#version 450 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec4 vertColor;
layout (location = 2) in uint vertId;
layout (location = 3) in uint vertParent;

layout (location = 0) uniform vec2 viewSize;

out vec2 fragTexCoord;
out vec4 fragColor;
out flat uint fragId;
out flat uint fragParent;

void main() {
	// Convert from pixels to NDC
	fragTexCoord = vec4((vertPos / viewSize) * 2 - 1, 0, 1);
	gl_Position.y = vec4(fragTexCoord.x, -fragTexCoord.y);
	fragId = vertId;
	fragParent = vertParent;
}
