#version 460 core

out vec4 fragColor;

in VertexData
{
	vec3 vNormal;
vec3 barycentric;
} vertexIn;

float edgeFactor()
{
	vec3 d = fwidth(vertexIn.barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * 1.5, vertexIn.barycentric);
	return min(min(a3.x, a3.y), a3.z);
}

void main(void)
{
	float ef = edgeFactor();
	vec4 faceColor = vec4(1.0, 0.0, 0.0, 1.0);
	vec4 edgeColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 color = mix(edgeColor, faceColor, ef);

	/*if (color.a < ef)
	{
	discard;
	}
	else
	{
	fragColor = color;
	}*/

	fragColor = color;
}