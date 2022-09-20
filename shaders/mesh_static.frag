#version 450 core

in vec2 fragTexCoord;
in uint fragDrawId;

out vec4 finalColor;

layout(std140, binding=2) uniform MaterialParameters {
	vec4 color;
	int tex;
};

layout(binding=0) uniform sampler2D textures[16];

void main() {
	finalColor = color * texture(textures[tex], fragTexCoord);
}
