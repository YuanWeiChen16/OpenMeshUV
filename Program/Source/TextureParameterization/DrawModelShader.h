#pragma once

#include <ShaderObject.h>

class DrawModelShader : public ShaderObject
{
public:
	DrawModelShader();
	~DrawModelShader();

	bool Init();
	void SetMVMat(const glm::mat4& mat);
	void SetPMat(const glm::mat4& mat);
	void SetNormalMat(const glm::mat3& mat);
	void SetUVRotMat(const glm::mat4& mat);
	void SetFaceColor(const glm::vec4& faceColor);
	void SetWireColor(const glm::vec4& faceColor);
	void SetTexcoord(const float& Tx, const float& Ty,const float& R);
	void SetNormalType(bool draw);
	void UseLighting(bool use);
	void DrawWireframe(bool draw);
	void DrawTexCoord(bool draw);
	void DrawTexture(bool draw);

private:
	GLuint um4pLocation;
	GLuint um4mvLocation;
	GLuint um3nLocation;
	GLuint um4uLocatoin;
	GLuint TexcoordX;
	GLuint TexcoordY;
	GLuint TexcoordR;
	GLuint NormalType;
	GLuint useLightingLocation;
	GLuint drawWireframeLocation;
	GLuint faceColorLocation;
	GLuint wireColorLocation;
	GLuint drawTexCoordLocation;
	GLuint drawTextureLocation;
};

