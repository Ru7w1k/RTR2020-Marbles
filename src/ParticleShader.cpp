// headers
#include "main.h"
#include "helper.h"
#include "logger.h"

#include "ParticleShader.h"


static GLuint ParticleShader;
static ParticleShaderUniforms* ParticleUniforms;

bool InitParticleShader()
{
	// create shader
	GLuint gVertexShaderObject;
	GLuint gGeometryShaderObject;
	GLuint gFragmentShaderObject;

	// provide source code to shader
	const GLchar* vertexShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 vPosition; \n" \
		"out int out_pID; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	gl_Position = vPosition; \n" \
		"	out_pID = gl_VertexID; \n" \
		"} \n";

	const GLchar* geometryShaderSourceCode = 
		"#version 450 core \n" \
		"layout (points) in; \n" \
		"layout (triangle_strip, max_vertices = 4) out; \n" \

		"in flat int out_pID[]; \n" \
		"out vec2 texCoord; \n" \
		"out int pID; \n" \

		"uniform mat4 u_mvpMatrix; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	float size = 0.18;" \
		"	pID  = out_pID[0]; \n" \

		"	gl_Position = u_mvpMatrix * (gl_in[0].gl_Position + vec4(size, size, 0.0, 0.0)); \n" \
		"	texCoord = vec2(1.0, 1.0); \n" \
		"	EmitVertex(); \n" \

		"	gl_Position = u_mvpMatrix * (gl_in[0].gl_Position + vec4(-size, size, 0.0, 0.0)); \n" \
		"	texCoord = vec2(0.0, 1.0); \n" \
		"	EmitVertex(); \n" \

		"	gl_Position = u_mvpMatrix * (gl_in[0].gl_Position + vec4(size, -size, 0.0, 0.0)); \n" \
		"	texCoord = vec2(1.0, 0.0); \n" \
		"	EmitVertex(); \n" \

		"	gl_Position = u_mvpMatrix * (gl_in[0].gl_Position + vec4(-size, -size, 0.0, 0.0)); \n" \
		"	texCoord = vec2(0.0, 0.0); \n" \
		"	EmitVertex(); \n" \

		"	EndPrimitive(); \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"in flat int pID; \n" \
		"in vec2 texCoord; \n" \
		"uniform sampler2D u_texSampler; \n" \
		"uniform float u_life[1000]; \n" \
		"uniform float u_lifespan; \n" \
		"uniform vec4 u_color; \n" \
		"out vec4 FragColor; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	vec4 color = texture(u_texSampler, texCoord); \n" \
		"	color.a   *= u_life[pID] * u_lifespan; \n" \
		"	FragColor  = color * u_color; \n" \
		"} \n";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode,
		GL_VERTEX_SHADER, "Particle Vertex"))
		return false;

	if (!CompileShader(gGeometryShaderObject, geometryShaderSourceCode,
		GL_GEOMETRY_SHADER, "Particle Geometry"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "Particle Fragment"))
		return false;

	//// shader program
	// create
	ParticleShader = glCreateProgram();

	// attach shaders
	glAttachShader(ParticleShader, gVertexShaderObject);
	glAttachShader(ParticleShader, gGeometryShaderObject);
	glAttachShader(ParticleShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(ParticleShader, RMC_ATTRIBUTE_POSITION, "vPosition");

	// link program
	if (!LinkProgram(ParticleShader, "ParticleShader")) return false;

	// post-linking retrieving uniform locations
	ParticleUniforms = (ParticleShaderUniforms*)malloc(sizeof(ParticleShaderUniforms));
	ParticleUniforms->mvpMatrixUniform  = glGetUniformLocation(ParticleShader, "u_mvpMatrix");
	ParticleUniforms->texSamplerUniform = glGetUniformLocation(ParticleShader, "u_texSampler");
	ParticleUniforms->lifeUniform       = glGetUniformLocation(ParticleShader, "u_life");
	ParticleUniforms->lifespanUniform   = glGetUniformLocation(ParticleShader, "u_lifespan");
	ParticleUniforms->colorUniform      = glGetUniformLocation(ParticleShader, "u_color");

	LogD("Particle Shader compiled..");
	return true;

}

void UninitParticleShader()
{
	if (ParticleShader)
	{
		DeleteProgram(ParticleShader);
		ParticleShader = 0;
		LogD("Particle Shader deleted..");
	}
}

ParticleShaderUniforms* UseParticleShader()
{
	glUseProgram(ParticleShader);
	return ParticleUniforms;
}


