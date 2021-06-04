#version 450 core

layout (location = 0) in vec2 vertPosition;

out vec2 fragTexCoord;

layout (location = 0) uniform vec2 scale;

void main() {
	gl_Position = vec4(vertPosition.x, vertPosition.y * scale.y, 0.0, 1.0);
	fragTexCoord = vertPosition.xy * 0.5;
	fragTexCoord.x /= scale.x;
}
