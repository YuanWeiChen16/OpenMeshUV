#pragma once
#include <ShaderObject.h>

class DrawPickingFaceShader: public ShaderObject
{
public:
	DrawPickingFaceShader();
	~DrawPickingFaceShader();

	bool Init();
	void SetMVMat(const GLfloat *value);
	void SetPMat(const GLfloat *value);

private:
	GLuint mvLocation;
	GLuint pLocation;
};

