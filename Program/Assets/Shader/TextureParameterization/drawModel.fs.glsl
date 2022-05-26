#version 430 core

out vec4 fragColor;

in VertexData
{
	vec3 vNormal;
	vec3 vViewPos;
	vec2 vTexCoord;
	vec3 vTanget;
	vec3 vBitanget;
	vec3 barycentric;
} vertexIn;

layout(binding=0) uniform sampler2D texImage;

layout(binding=1) uniform sampler2D NortexImage;

uniform bool useLighting;
uniform bool drawWireframe;
uniform bool drawTexture;
uniform vec4 faceColor;
uniform vec4 wireColor;
uniform float TexX;
uniform float TexY;
uniform float TexR;
uniform bool NormalType;
float edgeFactor()
{
	vec3 d = fwidth(vertexIn.barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * 1.5, vertexIn.barycentric);
	//vec3 a3 = smoothstep(vec3(0.0), d * 5, vertexIn.barycentric);
	return min(min(a3.x, a3.y), a3.z);
}

void main(void) 
{
	//vec4 faceColor = vec4(vec3(1.0) * lightIntense, 1.0);
	vec4 newFaceColor = faceColor;
	vec3 RealNormal = vertexIn.vNormal;
	if (drawTexture)
	{
		vec2 temptex = vec2(vertexIn.vTexCoord.x-0.5, vertexIn.vTexCoord.y-0.5);
		vec2 Texxx= vec2(0,0);
		Texxx.x = (cos(TexR)*temptex.x- sin(TexR)*temptex.y)+0.5+TexX;
		Texxx.y = (sin(TexR)*temptex.x+ cos(TexR)*temptex.y)+0.5+TexY;
		vec4 texColor = texture(texImage, Texxx);
		//newFaceColor = texColor;

		newFaceColor = vec4(0, vertexIn.vTexCoord.x,1,0);
		
		if(NormalType)
		{
			vec3 T = vertexIn.vTanget;
   			vec3 B = vertexIn.vBitanget;
			vec3 MyN = normalize(vertexIn.vNormal);
			mat3 TBN = mat3(T,B,MyN);
			vec3 N = texture(NortexImage, Texxx).rgb;
			N = N * 2.0 - 1.0;
			N = normalize(TBN * N);
			RealNormal = vec3(N.x,N.y,N.z);
		}
		
	}

	if (useLighting)
	{
		vec3 viewVector = -vertexIn.vViewPos;
		vec3 lightDir = vec3(0, 0, -1);

		vec3 L = -lightDir;
		vec3 V = normalize(viewVector);
		vec3 N = normalize(RealNormal);

		float ambient = 0.01;
		float diffuse = max(dot(N, L), 0);
		float specular = 0;
		if (diffuse > 0)
		{
			vec3 H = normalize(L + V);
			specular = pow(dot(N, H), 128);
		}

		newFaceColor = vec4(vec3(1.0) * (ambient + specular) + newFaceColor.xyz * diffuse, newFaceColor.a);
	}
	else
	{
		float R = vertexIn.vTexCoord.x;
		float GB = vertexIn.vTexCoord.y;

		newFaceColor = vec4(R, GB, 0, 1);
	}
	vec4 color = newFaceColor;
	//vec4 color = vec4(0,0.5,0,1);
	//vec4 color = faceColor;
	if(drawWireframe)
	{
		float ef = edgeFactor();
		color = mix(wireColor, newFaceColor, ef);
	}

	fragColor = color;
}