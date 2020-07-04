#version 450 core

in vec2 fragPosition;

out vec4 finalColor;

layout (location = 5) uniform sampler2D tex;


void main() {
	finalColor = texture(tex, fragPosition * 6);
}
