#version 450 core

in vec2 fragPosition;
layout (location = 5) uniform sampler2D tex; // Unused
out vec4 finalColor;



void main() {
	finalColor = texture(tex, fragPosition * 6);
}
