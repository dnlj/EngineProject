#version 450 core

#define MAX_BONES_PER_VERT 4

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTexCoord;
layout (location = 2) in uint vertBones[MAX_BONES_PER_VERT];
layout (location = 3) in float vertWeights[MAX_BONES_PER_VERT];
layout (location = 4) in uint vertDrawId;

layout (std140, binding=0) uniform ModelViewProjTransforms {
	mat4 mvps[1024];
};

layout (std140, binding=1) uniform Bones {
	mat4 bones[100];
};

out vec2 fragTexCoord;
out flat uint fragDrawId;

void main() {
	vec4 pos = vec4(0);
	vec4 vpos = vec4(vertPos, 1.0);

	for (int i = 0; i < MAX_BONES_PER_VERT; ++i) {
		pos += (bones[vertBones[i]] * vpos) * vertWeights[i];
	}

	gl_Position = mvps[vertDrawId] * pos;
	fragTexCoord = vertTexCoord;
	fragDrawId = vertDrawId;
}
