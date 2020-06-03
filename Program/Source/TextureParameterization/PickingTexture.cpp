#include "PickingTexture.h"


PickingTexture::PickingTexture()
{
}


PickingTexture::~PickingTexture()
{
	Clear();
}

bool PickingTexture::Init(int width, int height)
{
	Clear();
	
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fboColor);
	glBindTexture(GL_TEXTURE_2D, fboColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &fboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, fboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboDepth);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fboColor, 0);

	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	if (!Common::CheckFrameBufferStatus())
	{
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return Common::CheckGLError();
}

void PickingTexture::Clear()
{
	if (fboColor != 0)
	{
		glDeleteTextures(1, &fboColor);
	}

	if (fboDepth != 0)
	{
		glDeleteRenderbuffers(1, &fboDepth);
	}

	if (fbo != 0)
	{
		glDeleteFramebuffers(1, &fbo);
	}
}

void PickingTexture::Enable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void PickingTexture::Disable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint PickingTexture::ReadTexture(unsigned int x, unsigned int y)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	GLuint data;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return data;
}