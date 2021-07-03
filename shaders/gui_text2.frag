#version 450 core

in vec2 fragTexCoord;

layout (location = 0) out vec4 finalColor;

layout (location = 1) uniform sampler2D tex;

void main() {
	finalColor = texture(tex, fragTexCoord);
}
