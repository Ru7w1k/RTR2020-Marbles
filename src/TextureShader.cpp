#include "main.h"
#include "helper.h"
#include "logger.h"

#include "TextureShader.h"

static GLuint TextureShader;
static TextureShaderUniforms* TextureUniforms;

bool InitTextureShader()
{
	// create shader
	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	// provide source code to shader
	const GLchar* vertexShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 vPosition; \n" \
		"in vec2 vTexture0; \n" \

		"uniform mat4 u_mvpMatrix; \n" \

		"out vec2 out_Texture0; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	gl_Position = u_mvpMatrix * vPosition; \n" \
		"	out_Texture0 = vTexture0; \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in vec2 out_Texture0; \n" \
		"out vec4 FragColor; \n" \

		"uniform sampler2D u_Sampler; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	FragColor = texture(u_Sampler, out_Texture0); \n" \
		"} \n";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Texture Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Texture Fragment"))
		return false;

	//// shader program
	// create
	TextureShader = glCreateProgram();

	// attach shaders
	glAttachShader(TextureShader, gVertexShaderObject);
	glAttachShader(TextureShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(TextureShader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(TextureShader, RMC_ATTRIBUTE_TEXTURE0, "vTexture0");

	// link program
	if (!LinkProgram(TextureShader, "TextureShader")) return false;

	// post-linking retrieving uniform locations
	TextureUniforms = (TextureShaderUniforms*)malloc(sizeof(TextureUniforms));
	TextureUniforms->mvpMatrixUniform = glGetUniformLocation(TextureShader, "u_mvpMatrix");
	TextureUniforms->samplerUniform = glGetUniformLocation(TextureShader, "u_Sampler");

	LogD("Color Shader compiled..");
	return true;
}

void UninitTextureShader()
{
	if (TextureUniforms)
	{
		free(TextureUniforms);
	}

	if (TextureShader)
	{
		DeleteProgram(TextureShader);
		TextureShader = 0;
		LogD("Color Shader deleted..");
	}
}

TextureShaderUniforms* UseTextureShader()
{
	glUseProgram(TextureShader);
	return TextureUniforms;
}
