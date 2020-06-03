#include "DrawPickingFaceShader.h"
#include "ResourcePath.h"


DrawPickingFaceShader::DrawPickingFaceShader()
{
}


DrawPickingFaceShader::~DrawPickingFaceShader()
{
}

bool DrawPickingFaceShader::Init()
{
	if (!ShaderObject::Init())
	{
		return false;
	}

	if (!AddShader(GL_VERTEX_SHADER, ResourcePath::shaderPath + "drawPickingFace.vs.glsl"))
	{
		return false;
	}

	if (!AddShader(GL_GEOMETRY_SHADER, ResourcePath::shaderPath + "drawPickingFace.gs.glsl"))
	{
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, ResourcePath::shaderPath + "drawPickingFace.fs.glsl"))
	{
		return false;
	}

	if (!Finalize())
	{
		printf("Error %s\n", glewGetErrorString(glGetError()));
		puts("DrawPickingFaceShader error");
		return false;
	}

	mvLocation = GetUniformLocation("um4mv");
	if (mvLocation == -1)
	{
		puts("Get uniform loaction error: um4mv");
		return false;
	}

	pLocation = GetUniformLocation("um4p");
	if (pLocation == -1)
	{
		puts("Get uniform loaction error: um4p");
		return false;
	}

	return true;
}

void DrawPickingFaceShader::SetMVMat(const GLfloat *value)
{
	glUniformMatrix4fv(mvLocation, 1, GL_FALSE, value);
}

void DrawPickingFaceShader::SetPMat(const GLfloat *value)
{
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, value);
}