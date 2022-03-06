#version 450 core

in vec2 fragTexCoord;

layout (location = 0) out vec4 finalColor;

layout (location = 0) uniform vec2 viewSize;
layout (location = 2) uniform sampler2D glyphTex;

void main() {
	finalColor = vec4(1.0, 1.0, 1.0, texture(glyphTex, fragTexCoord).r);
	//finalColor = vec4(0,1,0,1);
}
