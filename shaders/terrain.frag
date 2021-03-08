#version 450 core

in vec2 fragPosition;
flat in int fragTexLayer;

out vec4 finalColor;

layout (location = 4) uniform sampler2DArray tex;

void main() {
	finalColor = texture(tex, vec3(fragPosition * 6, fragTexLayer));
}
