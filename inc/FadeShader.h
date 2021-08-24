#pragma once

#include "main.h"

typedef struct _FadeShaderUniforms
{
	GLint fade;

} FadeShaderUniforms;

bool InitFadeShader();
void UninitFadeShader();

FadeShaderUniforms* UseFadeShader();
