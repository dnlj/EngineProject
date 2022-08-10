#version 450 core

out vec4 finalColor;

layout (std140, binding=1) uniform MaterialParameters {
	vec4 color;
	int tex;
};

uniform sampler2D textures[16]; // TODO: bindless would make this easier

void main() {
	finalColor = texture(textures[tex], color.xy);
	finalColor.a = 1;
}
