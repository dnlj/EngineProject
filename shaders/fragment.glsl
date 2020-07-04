#version 450 core

in vec2 fragTexCoord;

out vec4 finalColor;

layout (location = 6) uniform sampler2D tex;

void main() {
	finalColor = texture(tex, fragTexCoord);
}