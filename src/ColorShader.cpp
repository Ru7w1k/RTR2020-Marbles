#include "main.h"
#include "helper.h"
#include "logger.h"

#include "ColorShader.h"

static GLuint ColorShader;
static ColorShaderUniforms* ColorUniforms;

bool InitColorShader()
{
	// create shader
	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	// provide source code to shader
	const GLchar* vertexShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 vPosition; \n" \
		"in vec4 vColor; \n" \

		"uniform mat4 u_mvpMatrix; \n" \

		"out vec4 out_Color; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	gl_Position = u_mvpMatrix * vPosition; \n" \
		"	out_Color = vColor; \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 out_Color; \n" \
		"out vec4 FragColor; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	FragColor = out_Color; \n" \
		"} \n";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Color Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Color Fragment"))
		return false;

	//// shader program
	// create
	ColorShader = glCreateProgram();

	// attach shaders
	glAttachShader(ColorShader, gVertexShaderObject);
	glAttachShader(ColorShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(ColorShader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(ColorShader, RMC_ATTRIBUTE_COLOR, "vColor");

	// link program
	if (!LinkProgram(ColorShader, "ColorShader")) return false;

	// post-linking retrieving uniform locations
	ColorUniforms = (ColorShaderUniforms*)malloc(sizeof(ColorUniforms));
	ColorUniforms->mvpMatrixUniform = glGetUniformLocation(ColorShader, "u_mvpMatrix");

	LogD("Color Shader compiled..");
	return true;
}

void UninitColorShader()
{
	if (ColorUniforms)
	{
		free(ColorUniforms);
	}

	if (ColorShader)
	{
		DeleteProgram(ColorShader);
		ColorShader = 0;
		LogD("Color Shader deleted..");
	}
}

ColorShaderUniforms* UseColorShader()
{
	glUseProgram(ColorShader);
	return ColorUniforms;
}
