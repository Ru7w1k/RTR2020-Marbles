// headers
#include "loading.h"

#include "helper.h"
#include "primitives.h"


static GLuint shader = 0;
static GLint texUniform = 0;
static GLint scaleUniform = 0;

static float s = 0.2f;

static GLuint tex = 0;

// functions
bool initLoadingScene(void)
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
		"	gl_Position =  vPosition; \n" \
		"	out_Texture0 = vTexture0; \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in vec2 out_Texture0; \n" \
		"out vec4 FragColor; \n" \

		"uniform sampler2D u_Tex; \n" \
		"uniform float u_Scale = 1.0; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	FragColor = texture(u_Tex, out_Texture0 * u_Scale); \n" \
		"} \n";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Loading Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Loading Fragment"))
		return false;

	//// shader program
	// create
	shader = glCreateProgram();

	// attach shaders
	glAttachShader(shader, gVertexShaderObject);
	glAttachShader(shader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(shader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(shader, RMC_ATTRIBUTE_TEXTURE0, "vTexture0");

	// link program
	if (!LinkProgram(shader, "Loading Shader")) 
		return false;

	// post-linking retrieving uniform locations
	texUniform = glGetUniformLocation(shader, "u_Tex");
	scaleUniform = glGetUniformLocation(shader, "u_Scale");

	tex = loadTexture("res\\textures\\noise.png");

	return true;
}

void displayLoadingScene(void)
{
	// draw a fullscreen quad with loading texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(texUniform, 0);
	glUniform1f(scaleUniform, s);
	DrawPlane();

	glUseProgram(0);
}

void resizeLoadingScene(int width, int height)
{
	// resize the texture 
}

void updateLoadingScene(float delta)
{
	s += 0.001f;
}

void uninitLoadingScene(void)
{
	// delete simple texture shader
	// release resource(?)
}

