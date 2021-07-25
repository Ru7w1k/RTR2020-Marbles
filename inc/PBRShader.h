#pragma once

#include "main.h"
#include "material.h"

typedef struct _PBRShaderUniforms
{
	GLint mMatrixUniform;
	GLint vMatrixUniform;
	GLint pMatrixUniform;
	GLint boneMatrixUniform;

	GLint lightPosUniform;
	GLint lightColUniform;
	GLint cameraPosUniform;

} PBRShaderUniforms;

bool InitPBRShader();
void UninitPBRShader();

PBRShaderUniforms* UsePBRShader();
