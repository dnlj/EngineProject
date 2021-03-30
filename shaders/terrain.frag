#version 450 core

in vec2 fragPosition;
in float fragTexLayer;

out vec4 finalColor;

layout (location = 4) uniform sampler2DArray tex;

void main() {
	finalColor = texture(tex, vec3(fragPosition * 4, fragTexLayer));
}
