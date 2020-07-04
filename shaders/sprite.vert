#version 450 core

layout (location = 0) in vec2 vertPosition;
layout (location = 1) in vec2 vertTexCoord;
layout (location = 2) in mat4 instMVP;

out vec2 fragTexCoord;


void main() {
	gl_Position = instMVP * vec4(vertPosition, 0.0, 1.0);
	fragTexCoord = vertTexCoord;
}
