#pragma once

#include "main.h"


typedef struct _framebufferParams
{
	int width;
	int height;
	int nColors;

} FramebufferParams;


typedef struct _framebuffer
{
	GLuint fbo;
	
	GLuint colorTex[8];
	int nColorTex;

	GLuint depthTex;

} Framebuffer;

Framebuffer* CreateFramebuffer(FramebufferParams *params);
void ResizeFramebuffer(Framebuffer *fb, int width, int height);
void DeleteFramebuffer(Framebuffer* fb);

void DrawFramebuffer(Framebuffer* fb, int colorIndex);


