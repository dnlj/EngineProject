#version 450 core

out vec4 finalColor;

layout (std140, binding=1) uniform MaterialParameters {
	vec4 color;
};

void main() {
	finalColor = color;
}
