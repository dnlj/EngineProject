#version 450 core

#define MAX_BONES_PER_VERT 4
#define INVALID_BONE 0xFF

layout (location = 0) in vec3 vertPos;
layout (location = 1) in uint vertBones[MAX_BONES_PER_VERT];
layout (location = 2) in float vertWeights[MAX_BONES_PER_VERT];

layout (location = 0) uniform mat4 mvp;

layout (std140, binding=0) uniform Bones {
	mat4 bones[100]; // TODO: what is a good size?
};

out vec4 fragColor;

void main() {
	vec4 pos = vec4(0);
	vec4 vpos = vec4(vertPos, 1.0);


	int i = 0;
	for (; i < MAX_BONES_PER_VERT; ++i) {
		if (vertWeights[i] == 0) { break; }
		pos += (bones[vertBones[i]] * vpos) * vertWeights[i];
	}

	--i;

	if (i == 0) { fragColor = vec4(1,0,0,1); }
	else if (i == 1) { fragColor = vec4(0,1,0,1); }
	else if (i == 2) { fragColor = vec4(0,0,1,1); }
	else if (i == 3) { fragColor = vec4(1,1,0,1); }

	gl_Position = mvp * pos;
}
