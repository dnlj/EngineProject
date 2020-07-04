#version 450 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec3 vertColor;

out vec3 fragColor;

layout (location = 2) uniform mat4 pv;

void main() {
	gl_Position = pv * vec4(vertPos, 0.0, 1.0);
	fragColor = vertColor;
}
