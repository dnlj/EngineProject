#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices=4) out;

in vec2 geomSize[1];
in vec2 geomTexCoord[1];
in vec3 geomOffset[1];

out vec2 fragTexCoord;

const float s = 0.03;

void main() {
	vec4 geomPos = gl_in[0].gl_Position;
	vec2 glyphSize = geomSize[0];
	vec2 glyphTexCoord = geomTexCoord[0];

	glyphSize.y = -glyphSize.y; // TODO: is this correct?

	gl_Position = geomPos;
	fragTexCoord = vec2(0.0, 0.0);
	EmitVertex();

	gl_Position = geomPos + vec4(0,glyphSize.y,0,0);
	//gl_Position = geomPos + vec4(0,s,0,0);
	fragTexCoord = vec2(0.0, 0.0);
	EmitVertex();
	
	gl_Position = geomPos + vec4(glyphSize.x,0,0,0);
	//gl_Position = geomPos + vec4(s,0,0,0);
	fragTexCoord = vec2(0.0, 0.0);
	EmitVertex();

	gl_Position = geomPos + vec4(glyphSize,0,0);
	//gl_Position = geomPos + vec4(vec2(s,s),0,0);
	fragTexCoord = vec2(0.0, 0.0);
	EmitVertex();

	EndPrimitive();
}
