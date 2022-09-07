#version 450 core

#define MAX_BONES_PER_VERT 4
#define INVALID_BONE 0xFF

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTexCoord;
layout (location = 2) in uint vertBones[MAX_BONES_PER_VERT];
layout (location = 3) in float vertWeights[MAX_BONES_PER_VERT];

layout (location = 0) uniform mat4 mvp;

layout (std140, binding=0) uniform Bones {
	mat4 bones[100]; // TODO: what is a good size?
};

out vec2 fragTexCoord;

void main() {
	vec4 pos = vec4(0);
	vec4 vpos = vec4(vertPos, 1.0);

	for (int i = 0; i < MAX_BONES_PER_VERT; ++i) {
		pos += (bones[vertBones[i]] * vpos) * vertWeights[i];
	}

	gl_Position = mvp * pos;
	fragTexCoord = vertTexCoord;
}
