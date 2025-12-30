#version 450 core

in vec2 fragPosition; // Fragment position in meters.
in float fragTexLayer;

out vec4 finalColor;

layout (location = 4) uniform sampler2DArray tex;

void main() {
	// TODO: programmatic shader constants based on common.hpp
	// pixelsPerMeter / res = 20 / 64
	finalColor = texture(tex, vec3(fragPosition * (20.0/64.0), fragTexLayer));
}
