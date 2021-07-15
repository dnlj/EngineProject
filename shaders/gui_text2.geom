#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices=4) out;

in vec2 geomSize[1];
in vec2 geomTexSize[1];
in vec3 geomTexOffset[1];

out vec2 fragTexCoord;

void main() {
	vec4 geomPos = gl_in[0].gl_Position;
	vec2 glyphSize = geomSize[0];
	vec2 glyphTexSize = geomTexSize[0];
	vec3 glyphTexOffset = geomTexOffset[0];

	glyphSize.y = -glyphSize.y;

	gl_Position = geomPos;
	fragTexCoord = glyphTexOffset.xy;
	EmitVertex();

	gl_Position = geomPos + vec4(0,glyphSize.y,0,0);
	fragTexCoord = glyphTexOffset.xy + vec2(0, glyphTexSize.y);
	EmitVertex();
	
	gl_Position = geomPos + vec4(glyphSize.x,0,0,0);
	fragTexCoord = glyphTexOffset.xy + vec2(glyphTexSize.x, 0);
	EmitVertex();

	gl_Position = geomPos + vec4(glyphSize,0,0);
	fragTexCoord = glyphTexOffset.xy + glyphTexSize;
	EmitVertex();

	EndPrimitive();
}
