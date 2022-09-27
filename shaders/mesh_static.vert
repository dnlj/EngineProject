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

out vec2 fragTexCoord;
out flat uint fragDrawId;

void main() {
	gl_Position = mvps[vertDrawId] * vec4(vertPos, 1);
	fragTexCoord = vertTexCoord;
	fragDrawId = vertDrawId;
}
