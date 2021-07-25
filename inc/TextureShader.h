#pragma once

#include "main.h"

typedef struct _TextureShaderUniforms
{
	GLint mvpMatrixUniform;
	GLint samplerUniform;

} TextureShaderUniforms;

bool InitTextureShader();
void UninitTextureShader();

TextureShaderUniforms* UseTextureShader();
