// headers
#include "main.h"
#include "logger.h"

#include "framebuffer.h"

#include "primitives.h"
#include "TextureShader.h"

#include <vector>
using namespace std;

Framebuffer* CreateFramebuffer(FramebufferParams* params)
{
	vector<GLenum> drawBuffers;

	Framebuffer* f = (Framebuffer*)malloc(sizeof(Framebuffer));
	memset(f, 0, sizeof(Framebuffer));

	glGenFramebuffers(1, &(f->fbo));
	glBindFramebuffer(GL_FRAMEBUFFER, f->fbo);

	// color textures
	f->nColorTex = params->nColors;
	for (int i = 0; i < f->nColorTex; i++)
	{
		glGenTextures(1, &(f->colorTex[i]));
		glBindTexture(GL_TEXTURE_2D, f->colorTex[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, params->width, params->height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, f->colorTex[i], 0);
		drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	// depth texture
	glGenRenderbuffers(1, &(f->depthTex));
	glBindRenderbuffer(GL_RENDERBUFFER, f->depthTex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, params->width, params->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, f->depthTex);

	glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LogE("Framebuffer incomplete!");
		
		glDeleteTextures(1, &(f->depthTex));
		glDeleteTextures(params->nColors, f->colorTex);
		glDeleteFramebuffers(1, &(f->fbo));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return NULL;
	}

	LogD("Framebuffer created successfully..");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	drawBuffers.clear();
	return f;
}

void ResizeFramebuffer(Framebuffer* f, int width, int height)
{
	DeleteFramebuffer(f);

	FramebufferParams params;
	params.width = width;
	params.height = height;
	params.nColors = f->nColorTex;
	f = CreateFramebuffer(&params);
}

void DeleteFramebuffer(Framebuffer* fb)
{
	if (fb)
	{
		glDeleteTextures(1, &(fb->depthTex));
		glDeleteTextures(fb->nColorTex, fb->colorTex);
		glDeleteFramebuffers(1, &(fb->fbo));
	}
}

void DrawFramebuffer(Framebuffer* fb)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb->colorTex[0]);

	TextureShaderUniforms *u = UseTextureShader();
	glUniformMatrix4fv(u->mvpMatrixUniform, 1, GL_FALSE, mat4::identity());
	glUniform1i(u->samplerUniform, 0);

	DrawPlane();
	glUseProgram(0);
}
