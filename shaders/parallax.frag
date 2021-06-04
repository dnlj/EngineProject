#version 450 core

in vec2 fragTexCoord;

out vec4 finalColor;

layout (location = 1) uniform sampler2D tex;
layout (location = 2) uniform float xoff;


void main() {
	finalColor = texture(tex, vec2(fragTexCoord.x + xoff, fragTexCoord.y));

	if (finalColor.a < 0.5) {
		discard;
	}
}
