#version 450 core

layout (location = 0) in vec2 vertPosition;
layout (location = 1) in vec2 vertInstScale;
layout (location = 2) in float vertInstXOff;

out vec2 fragTexCoord;

void main() {
	gl_Position = vec4(vertPosition.x, vertPosition.y * vertInstScale.y, 0.0, 1.0);
	fragTexCoord = vertPosition.xy * 0.5;
	fragTexCoord.x /= vertInstScale.x;
	fragTexCoord.x += vertInstXOff;
}
