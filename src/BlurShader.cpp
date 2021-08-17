#include "main.h"
#include "helper.h"
#include "logger.h"

#include "BlurShader.h"

static GLuint BlurShader;
static BlurShaderUniforms* BlurUniforms;

bool InitBlurShader()
{
	// create shader
	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	// provide source code to shader
	const GLchar* vertexShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 vPosition; \n" \
		"in vec2 vTexcoord; \n" \

		"out vec2 TexCoord; \n" \

		"void main(void) \n" \
		"{" \
		"   TexCoord = vTexcoord; \n" \
		"   gl_Position = vPosition; \n" \
		"}";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in vec2 TexCoord; \n" \
		"out vec4 FragColor; \n" \

		"uniform sampler2D image; \n" \
		"uniform bool horizontal; \n" \
		"const float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162); \n" \

		"void main(void)" \
		"{" \
		"	vec2 tex_offset = 1.0 / textureSize(image, 0); // get size of single texel \n" \
		"	vec3 result = texture(image, TexCoord).rgb * weight[0]; \n" \

		"	if (horizontal) \n" \
		"	{ \n" \
		"		for(int i = 0; i < 5; i++) \n" \
		"		{ \n" \
		"			result += texture(image, TexCoord + vec2(tex_offset.x * i*2, 0.0)).rgb * weight[i]; \n" \
		"			result += texture(image, TexCoord - vec2(tex_offset.x * i*2, 0.0)).rgb * weight[i]; \n" \
		"		} \n" \
		"	} \n" \

		"	else \n" \
		"	{ \n" \
		"		for(int i = 0; i < 5; i++) \n" \
		"		{ \n" \
		"			result += texture(image, TexCoord + vec2(0.0, tex_offset.y * i*2)).rgb * weight[i]; \n" \
		"			result += texture(image, TexCoord - vec2(0.0, tex_offset.y * i*2)).rgb * weight[i]; \n" \
		"		} \n" \
		"	} \n" \

		"	FragColor = vec4(result, 1.0); \n" \
		"}";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Blur Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Blur Fragment"))
		return false;

	//// shader program
	// create
	BlurShader = glCreateProgram();

	// attach shaders
	glAttachShader(BlurShader, gVertexShaderObject);
	glAttachShader(BlurShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(BlurShader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(BlurShader, RMC_ATTRIBUTE_TEXTURE0, "vTexcoord");

	// link program
	if (!LinkProgram(BlurShader, "BlurShader")) return false;

	// post-linking retrieving uniform locations
	BlurUniforms = (BlurShaderUniforms*)malloc(sizeof(BlurShaderUniforms));
	BlurUniforms->image = glGetUniformLocation(BlurShader, "image");
	BlurUniforms->horizontal = glGetUniformLocation(BlurShader, "horizontal");

	LogD("Blur Shader compiled..");
	return true;
}

void UninitBlurShader()
{
	if (BlurUniforms)
	{
		free(BlurUniforms);
	}

	if (BlurShader)
	{
		DeleteProgram(BlurShader);
		BlurShader = 0;
		LogD("Color Shader deleted..");
	}
}

BlurShaderUniforms* UseBlurShader()
{
	glUseProgram(BlurShader);
	return BlurUniforms;
}
