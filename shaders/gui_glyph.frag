#version 450 core

in vec2 fragTexCoord;
in vec4 fragColor;

layout (location = 0) out vec4 finalColor;

layout (location = 1) uniform sampler2D glyphTex;

void main() {
	finalColor = fragColor;
	finalColor.a *= texture(glyphTex, fragTexCoord).r;
}
