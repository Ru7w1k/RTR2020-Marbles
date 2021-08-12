#pragma once

#include "main.h"

typedef struct _BloomShaderUniforms
{
	GLint tex1;
	GLint tex2;

} BloomShaderUniforms;

bool InitBloomShader();
void UninitBloomShader();

BloomShaderUniforms* UseBloomShader();
