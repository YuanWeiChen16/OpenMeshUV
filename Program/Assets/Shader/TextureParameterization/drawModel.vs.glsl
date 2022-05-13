#version 430 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat3 um3n;
uniform mat4 um4mv;
uniform mat4 um4p;
uniform mat4 um4u;
uniform bool useLighting;
uniform bool drawTexCoord;

out VertexData
{
	vec3 vNormal;
	vec3 vViewPos;
	vec2 vTexCoord;
	vec4 VRealPos;
	mat4 MV;
} vertexOut;

void main()
{
	vec4 pos;
	if (drawTexCoord)
	{
		vec2 offset = vec2(-0.5, -0.5);
		//pos = vec4(vertex, 1.0);
		pos = um4u * vec4(texCoord + offset, 0.0, 1.0);
		//pos = vec4(vertex, 1.0);
	}
	else
	{
		pos = vec4(vertex, 1.0);
	}

	vec4 newTexCoord = vec4(texCoord.x, texCoord.y, 0.0, 1.0);
	vec4 viewModelPos = um4mv * vec4(vertex, 1.0);
	vertexOut.vNormal = normalize(um3n * (normal));
	/*if (!useLighting)
	{
		vertexOut.vNormal = vec3(0, -1, 0);
	}*/
	//vertexOut.vNormal = normalize(um3n * (normal));
	vertexOut.vViewPos = viewModelPos.xyz;
	vertexOut.vTexCoord = texCoord;
	vertexOut.MV = um4mv;
	vertexOut.VRealPos =  pos;
	gl_Position = um4p * um4mv * pos;
}
