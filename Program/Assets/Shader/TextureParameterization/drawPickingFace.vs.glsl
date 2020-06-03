#version 460 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;

uniform mat4 um4mv;
uniform mat4 um4p;

out VertexData
{
	vec3 normal;
} vertexOut;

void main()
{
	vertexOut.normal = normal;
	gl_Position = um4p * um4mv * vec4(vertex, 1.0);
}
