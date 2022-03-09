#version 450 core

in vec4 fragColor;
in vec2 fragTexCoord;
in vec2 fragClipTexCoord;

layout (location = 0) out vec4 finalColor;
// TODO: rm - layout (location = 1) out float finalId;

layout (location = 1) uniform sampler2D clipTex; // TODO: rm 
layout (location = 2) uniform sampler2D tex;

void main() {
	finalColor = fragColor * texture(tex, fragTexCoord);
}
