#version 450 core

in vec4 fragColor;
in vec2 fragTexCoord;
in float fragId;
in float fragParent;

layout (location = 0) out vec4 finalColor;
layout (location = 1) out float finalId;

layout (location = 1) uniform sampler2D clipTex;

void main() {
	const float under = texture(clipTex, fragTexCoord).r;
	if (under != fragParent) {
		discard; // TODO: is discard performant? could also just do:
		//finalId = under;
		//finalColor = vec4(0,0,0,0);
	} else {
		finalColor = fragColor;
		finalId = fragId;
	}
}
