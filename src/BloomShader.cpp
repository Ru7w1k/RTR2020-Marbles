#include "main.h"
#include "helper.h"
#include "logger.h"

#include "BloomShader.h"

static GLuint BloomShader;
static BloomShaderUniforms* BloomUniforms;

bool InitBloomShader()
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
		"	gl_Position  = vPosition; \n" \
		"	out_Texture0 = vTexture0; \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in vec2 out_Texture0; \n" \
		"out vec4 FragColor; \n" \

		"uniform sampler2D tex1; \n" \
		"uniform sampler2D tex2; \n" \

		"uniform bool bloom = true; \n" \

		"void main(void)" \
		"{" \
		"	const float gamma = 2.2; \n" \
		"	const float exposure = 1.0; \n" \
		"	vec3 hdrColor =  texture(tex1, out_Texture0).rgb; \n" \
		"	vec3 bloomColor =  texture(tex2, out_Texture0).rgb; \n" \
		"	if (bloom) hdrColor += bloomColor; \n" \

		"	FragColor = vec4(hdrColor, 1.0); \n" \
		"}";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Texture Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Texture Fragment"))
		return false;

	//// shader program
	// create
	BloomShader = glCreateProgram();

	// attach shaders
	glAttachShader(BloomShader, gVertexShaderObject);
	glAttachShader(BloomShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(BloomShader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(BloomShader, RMC_ATTRIBUTE_TEXTURE0, "vTexture0");

	// link program
	if (!LinkProgram(BloomShader, "BloomShader")) return false;

	// post-linking retrieving uniform locations
	BloomUniforms = (BloomShaderUniforms*)malloc(sizeof(BloomShaderUniforms));
	BloomUniforms->tex1 = glGetUniformLocation(BloomShader, "tex1");
	BloomUniforms->tex2 = glGetUniformLocation(BloomShader, "tex2");

	LogD("Bloom Shader compiled..");
	return true;
}

void UninitBloomShader()
{
	if (BloomUniforms)
	{
		free(BloomUniforms);
	}

	if (BloomShader)
	{
		DeleteProgram(BloomShader);
		BloomShader = 0;
		LogD("Bloom Shader deleted..");
	}
}

BloomShaderUniforms* UseBloomShader()
{
	glUseProgram(BloomShader);
	return BloomUniforms;
}
