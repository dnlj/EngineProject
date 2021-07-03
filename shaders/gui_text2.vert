#version 450 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in uint vertIndex;

layout (location = 0) uniform vec2 viewSize;

out vec2 geomSize;
out vec2 geomTexCoord;
out vec3 geomOffset;

struct GlyphData {
	vec2 size;
	vec3 offset;
};

layout(std430, binding = 0) readonly restrict buffer GlyphDataBuffer {
	GlyphData glyphData[];
};

void main() {
	// TODO: just do `2 / viewSize` on cpu so we can just mult and dont have to / view * 2 here
	// Convert from pixels to NDC
	gl_Position = vec4((vertPos / viewSize) * 2 - 1, 0, 1);
	gl_Position.y = -gl_Position.y;

	const vec2 glyphSize = glyphData[vertIndex].size;
	geomSize = (glyphSize / viewSize) * 2;
	geomTexCoord = glyphSize / 4096; // TODO: dont hardcode?
	geomOffset = glyphData[vertIndex].offset;
}
