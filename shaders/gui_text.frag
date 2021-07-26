#version 450 core

in vec2 fragTexCoord;

layout (location = 0) out vec4 finalColor;

layout (location = 2) uniform sampler2D tex;

void main() {
	finalColor = vec4(1.0, 1.0, 1.0, texture(tex, fragTexCoord).r);
	//finalColor = vec4(1,0,0,1);
}
