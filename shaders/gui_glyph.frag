#version 450 core

in vec2 fragTexCoord;
in float fragParent;

layout (location = 0) out vec4 finalColor;

layout (location = 0) uniform vec2 viewSize;
layout (location = 1) uniform sampler2D clipTex;
layout (location = 2) uniform sampler2D glyphTex;

void main() {
	const float under = texture(clipTex, gl_FragCoord.xy / viewSize).r;
	if (under != fragParent) { discard; }
	finalColor = vec4(1.0, 1.0, 1.0, texture(glyphTex, fragTexCoord).r);
	//finalColor = vec4(0,1,0,1);
}
