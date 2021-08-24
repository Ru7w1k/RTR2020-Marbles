#include "main.h"
#include "helper.h"
#include "logger.h"

#include "FadeShader.h"

static GLuint FadeShader;
static FadeShaderUniforms* FadeUniforms;

bool InitFadeShader()
{
	// create shader
	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	// provide source code to shader
	const GLchar* vertexShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 vPosition; \n" \
		"in vec2 vTexture0; \n" \

		"out vec2 out_Texture0; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	gl_Position = vPosition; \n" \
		"	out_Texture0 = vTexture0; \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in vec2 out_Texture0; \n" \
		"out vec4 FragColor; \n" \

		"uniform float u_fade; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	FragColor = vec4(0.0f, 0.0f, 0.0f, u_fade); \n" \
		"} \n";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Fade Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Fade Fragment"))
		return false;

	//// shader program
	// create
	FadeShader = glCreateProgram();

	// attach shaders
	glAttachShader(FadeShader, gVertexShaderObject);
	glAttachShader(FadeShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(FadeShader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(FadeShader, RMC_ATTRIBUTE_TEXTURE0, "vTexture0");

	// link program
	if (!LinkProgram(FadeShader, "FadeShader")) return false;

	// post-linking retrieving uniform locations
	FadeUniforms = (FadeShaderUniforms*)malloc(sizeof(FadeShaderUniforms));
	FadeUniforms->fade = glGetUniformLocation(FadeShader, "u_fade");

	LogD("Fade Shader compiled..");
	return true;
}

void UninitFadeShader()
{
	if (FadeUniforms)
	{
		free(FadeUniforms);
	}

	if (FadeShader)
	{
		DeleteProgram(FadeShader);
		FadeShader = 0;
		LogD("Fade Shader deleted..");
	}
}

FadeShaderUniforms* UseFadeShader()
{
	glUseProgram(FadeShader);
	return FadeUniforms;
}
