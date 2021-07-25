#pragma once

#include "main.h"

typedef struct _ColorShaderUniforms
{
	GLint mvpMatrixUniform;

} ColorShaderUniforms;

bool InitColorShader();
void UninitColorShader();

ColorShaderUniforms* UseColorShader();
