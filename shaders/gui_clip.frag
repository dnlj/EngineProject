#version 450 core

in vec2 fragTexCoord;
in vec4 fragColor;
in uint fragId;
in uint fragParent

out uint finalId;
out uint finalColor;

layout (location = 1) uniform usampler2D clipTex;

void main() {
	uint under = texture(clipTex, fragTexCoord);
	if (under != fragParent) {
		discard; // TODO: is discard performant? could also just do:
		// finalId = under;
		// finalColor = vec4(0,0,0,0);
	} else {
		finalId = fragId;
		finalColor = fragColor;
	}
}
