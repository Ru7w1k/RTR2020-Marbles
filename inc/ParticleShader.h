#pragma once
#include "main.h"

typedef struct _ParticleShaderUniforms
{
	GLint mvpMatrixUniform;
	GLint texSamplerUniform;
	GLint lifeUniform;
	GLint lifespanUniform;
	GLint colorUniform;

} ParticleShaderUniforms;

bool InitParticleShader();
void UninitParticleShader();

ParticleShaderUniforms* UseParticleShader();
