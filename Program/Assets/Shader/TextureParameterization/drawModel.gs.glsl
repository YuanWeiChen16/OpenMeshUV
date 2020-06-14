#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertexData
{
	vec3 vNormal;
	vec3 vViewPos;
	vec2 vTexCoord;
	vec3 VRealPos;
	mat4 MV;
} vertexIn[];

out VertexData
{
	vec3 vNormal;
	vec3 vViewPos;
	vec2 vTexCoord;
	vec3 vTanget;
	vec3 vBitanget;
	vec3 barycentric;
} vertexOut;

void main()
{

	vec3 edge1 =  vec3(vertexIn[0].VRealPos) - vec3(vertexIn[1].VRealPos);
	vec3 edge2 =  vec3(vertexIn[2].VRealPos) - vec3(vertexIn[1].VRealPos);
	vec2 deltaUV1 = vertexIn[0].vTexCoord-vertexIn[1].vTexCoord;
	vec2 deltaUV2 = vertexIn[2].vTexCoord-vertexIn[1].vTexCoord;
	
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	vec3 tangent;
	vec3 bitangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	
	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

	vec3 tangent1 = normalize(vec3(vertexIn[0].MV*vec4(tangent,0.0)));
	vec3 bitangent1 = normalize(vec3(vertexIn[0].MV*vec4(bitangent,0.0)));
	vec3 tangent2 = normalize(vec3(vertexIn[1].MV*vec4(tangent,0.0)));
	vec3 bitangent2 = normalize(vec3(vertexIn[1].MV*vec4(bitangent,0.0)));
	vec3 tangent3 = normalize(vec3(vertexIn[2].MV*vec4(tangent,0.0)));
	vec3 bitangent3 = normalize(vec3(vertexIn[2].MV*vec4(bitangent,0.0)));


	vertexOut.vNormal = vertexIn[0].vNormal;
	vertexOut.vViewPos = vertexIn[0].vViewPos;
	vertexOut.vTexCoord = vertexIn[0].vTexCoord;
	vertexOut.vTanget =  tangent1;
	vertexOut.vBitanget =  bitangent1;
	vertexOut.barycentric = vec3(1, 0, 0);
	gl_Position = gl_in[0].gl_Position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	vertexOut.vNormal = vertexIn[1].vNormal;
	vertexOut.vViewPos = vertexIn[1].vViewPos;
	vertexOut.vTexCoord = vertexIn[1].vTexCoord;
	vertexOut.vTanget = tangent2;
	vertexOut.vBitanget =  bitangent2;
	vertexOut.barycentric = vec3(0, 1, 0);
	gl_Position = gl_in[1].gl_Position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	vertexOut.vNormal = vertexIn[2].vNormal;
	vertexOut.vViewPos = vertexIn[2].vViewPos;
	vertexOut.vTexCoord = vertexIn[2].vTexCoord;
	vertexOut.vTanget = tangent3;
	vertexOut.vBitanget =  bitangent3;
	vertexOut.barycentric = vec3(0, 0, 1);
	gl_Position = gl_in[2].gl_Position;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	EndPrimitive();
}