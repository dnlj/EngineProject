#version 450 core

in vec4 fragColor;
in vec2 fragTexCoord;
in vec2 fragClipTexCoord;
in float fragId;
in float fragParent;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out float finalId;

layout (location = 1) uniform sampler2D clipTex;
layout (location = 2) uniform sampler2D tex;

void main() {
	const float under = texture(clipTex, fragClipTexCoord).r;
	if (under != fragParent) {
		discard; // TODO: is discard performant? could also just do:
		// TODO: if we do one of these also update the glyph shader
		//finalId = under;
		//finalColor = vec4(0,0,0,0);
	} else {
		finalColor = fragColor * texture(tex, fragClipTexCoord); // TODO: fragTexCoord
		finalId = fragId;
	}
}
