#version 450 core

layout (location = 0) in vec4 vertColor;
layout (location = 1) in vec2 vertPos;
layout (location = 2) in float vertId;
layout (location = 3) in float vertParent;

layout (location = 0) uniform vec2 viewSize;

out vec4 fragColor;
out vec2 fragTexCoord;
out float fragId;
out float fragParent;

void main() {
	// Convert from pixels to NDC
	fragTexCoord = vertPos / viewSize;
	fragTexCoord.y = 1 - fragTexCoord.y;
	gl_Position = vec4(fragTexCoord * 2 - 1, 0, 1);
	//gl_Position.y = -gl_Position.y;

	fragColor = vertColor;
	fragId = vertId;
	fragParent = vertParent;
}
