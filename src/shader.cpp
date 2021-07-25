// headers
#include "main.h"
#include "shader.h"

// shaders
#include "ColorShader.h"
#include "TextureShader.h"
#include "ParticleShader.h"
#include "PBRShader.h"

bool InitAllShaders()
{

	if (!InitColorShader())    return false;
	if (!InitTextureShader())  return false;
	if (!InitParticleShader()) return false;
	if (!InitPBRShader())      return false;

	return true;
}

void UninitAllShaders()
{
	UninitColorShader();
	UninitTextureShader();
	UninitParticleShader();
	UninitPBRShader();
	return;
}
