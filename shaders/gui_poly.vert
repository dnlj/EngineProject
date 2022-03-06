#version 450 core

// TODO: any benefit to packing these with `component` specifier?
layout (location = 0) in vec4 vertColor;
layout (location = 1) in vec2 vertTexCoord;
layout (location = 2) in vec2 vertPos;
layout (location = 3) in float vertId;
layout (location = 4) in float vertParent;

layout (location = 0) uniform vec2 viewSize;

out vec4 fragColor;
out vec2 fragTexCoord;
out vec2 fragClipTexCoord;
out float fragId;
out float fragParent;

void main() {
	// Convert from pixels to NDC
	fragClipTexCoord = vertPos / viewSize;
	fragClipTexCoord.y = 1 - fragClipTexCoord.y;
	gl_Position = vec4(fragClipTexCoord * 2 - 1, 0, 1);

	fragColor = vertColor;
	fragTexCoord = vertTexCoord;
	fragId = vertId;
	fragParent = vertParent;
}
