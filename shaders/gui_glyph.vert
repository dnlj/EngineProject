#version 450 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in uint vertIndex;

layout (location = 0) uniform vec2 viewScale; // = 2 / viewSize

out vec2 geomSize;
out vec2 geomTexSize;
out vec3 geomTexOffset;

struct GlyphData {
	vec2 size;
	vec3 offset;
};

layout(std430, binding = 0) readonly restrict buffer GlyphDataBuffer {
	GlyphData glyphData[];
};

void main() {
	gl_Position = vec4(vertPos * viewScale - 1, 0, 1);
	gl_Position.y = -gl_Position.y;

	const vec2 glyphSize = glyphData[vertIndex].size;
	geomSize = glyphSize * viewScale;
	geomTexSize = glyphSize / 4096; // TODO: dont hardcode?
	geomTexOffset = glyphData[vertIndex].offset / 4096;
}
