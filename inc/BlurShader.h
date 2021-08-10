#pragma once

#include "main.h"

typedef struct _BlurShaderUniforms
{
	GLint image;
	GLint horizontal;

} BlurShaderUniforms;

bool InitBlurShader();
void UninitBlurShader();

BlurShaderUniforms* UseBlurShader();
