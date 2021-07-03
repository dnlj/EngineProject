#version 450 core

in vec2 fragTexCoord;
in uint fragIndex;

layout (location = 0) out vec4 finalColor;

layout (location = 1) uniform sampler2D tex;

void main() {
	finalColor = texture(tex, fragTexCoord);
	finalColor = vec4(1,0,0,1);
	//finalColor = texture(tex, vec2(0,0));
}
