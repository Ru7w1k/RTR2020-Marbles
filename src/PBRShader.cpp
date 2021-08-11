#include "main.h"
#include "helper.h"
#include "logger.h"

#include "PBRShader.h"

static GLuint PBRShader;
static PBRShaderUniforms *PBRUniforms;

bool InitPBRShader()
{
	// create shader
	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	// provide source code to shader
	const GLchar* vertexShaderSourceCode =
		"#version 450 core \n" \

		"in vec4 vPosition; \n" \
		"in vec3 vNormal; \n" \
		"in vec2 vTexcoord; \n" \

		"uniform mat4 u_modelMatrix; \n" \
		"uniform mat4 u_viewMatrix; \n" \
		"uniform mat4 u_projectionMatrix; \n" \

		"out vec3 out_WorldPos; \n" \
		"out vec3 out_Normal; \n" \
		"out vec2 out_Texcoord; \n" \

		"void main (void) \n" \
		"{ \n" \
		"	out_Texcoord = vTexcoord; \n " \
		"	out_WorldPos = vec3(u_modelMatrix * vPosition); \n " \
		"	out_Normal = normalize(mat3(u_modelMatrix) * vNormal); \n " \

		"	gl_Position = u_projectionMatrix * u_viewMatrix * vec4(out_WorldPos, 1.0); \n" \
		"} \n";

	// provide source code to shader
	const GLchar* fragmentShaderSourceCode =
		"#version 450 core \n" \

		"const float PI = 3.14159265359; \n" \

		"layout (location = 0) out vec4 FragColor; \n" \
		"layout (location = 1) out vec4 BrightColor; \n" \

		"in vec3 out_WorldPos; \n" \
		"in vec3 out_Normal; \n" \
		"in vec2 out_Texcoord; \n" \

		"uniform sampler2D albedoMap; \n" \
		"uniform sampler2D normalMap; \n" \
		"uniform sampler2D metallicMap; \n" \
		"uniform sampler2D roughnessMap; \n" \
		"uniform sampler2D aoMap; \n" \

		"uniform vec3 lightPosition[10]; \n" \
		"uniform vec3 lightColor[10]; \n" \

		"uniform vec3 cameraPos; \n" \

		"uniform float alpha = 1.0; \n" \

		"vec3 getNormalFromMap() \n" \
		"{ \n" \
		"	vec3 tangentNormal = texture(normalMap, out_Texcoord).xyz * 2.0 - 1.0; \n" \

		"	vec3 Q1 = dFdx(out_WorldPos); \n" \
		"	vec3 Q2 = dFdy(out_WorldPos); \n" \
		"	vec2 st1 = dFdx(out_Texcoord); \n" \
		"	vec2 st2 = dFdy(out_Texcoord); \n" \

		"	vec3 N = normalize(out_Normal); \n" \
		"	vec3 T = normalize(Q1*st2.t - Q2*st1.t); \n" \
		"	vec3 B = -normalize(cross(N, T)); \n" \
		"	mat3 TBN = mat3(T, B, N); \n" \

		"	return normalize(TBN * tangentNormal); \n" \
		"} \n" \

		"float DistributionGGX(vec3 N, vec3 H, float roughness) \n" \
		"{ \n" \
		"	float a = roughness * roughness; \n" \
		"	float a2 = a * a; \n" \
		"	float NdotH = max(dot(N, H), 0.0); \n" \
		"	float NdotH2 = NdotH * NdotH; \n" \

		"	float nom = a2; \n" \
		"	float denom = (NdotH2 * (a2 - 1.0) + 1.0); \n" \
		"	denom = PI * denom * denom; \n" \

		"	return nom / denom; \n" \
		"} \n" \

		"float GeometrySchlickGGX(float NdotV, float roughness) \n" \
		"{ \n" \
		"	float r = roughness + 1.0; \n" \
		"	float k = (r * r) / 8.0; \n" \

		"	float nom = NdotV; \n" \
		"	float denom = NdotV * (1.0 - k) + k; \n" \

		"	return nom / denom; \n" \
		"} \n" \

		"float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) \n" \
		"{ \n" \
		"	float NdotV = max(dot(N, V), 0.0); \n" \
		"	float NdotL = max(dot(N, L), 0.0); \n" \

		"	float ggx2 = GeometrySchlickGGX(NdotV, roughness); \n" \
		"	float ggx1 = GeometrySchlickGGX(NdotL, roughness); \n" \

		"	return ggx1 * ggx2; \n" \
		"} \n" \

		"vec3 fresnelSchlick(float cosTheta, vec3 F0) \n" \
		"{ \n" \
		"	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); \n" \
		"} \n" \

		"void main (void) \n" \
		"{ \n" \
		"	vec3 albedo     = pow(texture(albedoMap, out_Texcoord).rgb, vec3(2.2)); \n" \
		"	float metallic  = texture(metallicMap, out_Texcoord).r; \n" \
		"	float roughness = texture(roughnessMap, out_Texcoord).r; \n" \
		"	float ao        = texture(aoMap, out_Texcoord).r; \n" \

		"	vec3 N = getNormalFromMap(); \n" \
		"	vec3 V = normalize(cameraPos - out_WorldPos); \n" \

		"	vec3 F0 = vec3(0.04); \n" \
		"	F0 = mix(F0, albedo, metallic); \n" \

		"	vec3 Lo = vec3(0.0); \n" \

		"	for(int i = 0; i < 9; i++) \n" \
		"	{ \n" \
		"		vec3 L = normalize(lightPosition[i] - out_WorldPos); \n" \
		"		vec3 H = normalize(V + L); \n" \
		"		float distance = length(lightPosition[i] - out_WorldPos); \n" \
		"		float attenuation = 1.0 / (distance * distance); \n" \
		"		vec3 radiance = 7.0f * lightColor[i] * attenuation; \n" \

		"		float NDF = DistributionGGX(N, H, roughness); \n" \
		"		float G   = GeometrySmith(N, V, L, roughness); \n" \
		"		vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0); \n" \

		"		vec3 nominator   = NDF * G * F; \n" \
		"		float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; \n" \
		"		vec3 specular = nominator / denominator; \n" \

		"		vec3 kS = F; \n" \
		"		vec3 kD = vec3(1.0) - kS; \n" \
		"		kD *= 1.0 - metallic; \n" \

		"		float NdotL = max(dot(N, L), 0.0); \n" \

		"		Lo += (kD * albedo / PI + specular) * radiance * NdotL; \n" \

		"	} \n" \

		"	vec3 ambient = vec3(0.01) * albedo * ao; \n" \

		"	vec3 color = ambient + Lo; \n" \

		"	color = color / (color + vec3(1.0)); \n" \
		"	color = pow(color, vec3(1.0 / 2.2)); \n" \

		"	FragColor = vec4(color, alpha); \n" \

		"	BrightColor = vec4(1.0); \n" \
		"} \n";

	// compile shaders
	if (!CompileShader(gVertexShaderObject, vertexShaderSourceCode, 
		GL_VERTEX_SHADER, "PBR Vertex"))
		return false;

	if (!CompileShader(gFragmentShaderObject, fragmentShaderSourceCode,
		GL_FRAGMENT_SHADER, "PBR Fragment"))
		return false;

	//// shader program
	// create
	PBRShader = glCreateProgram();

	// attach shaders
	glAttachShader(PBRShader, gVertexShaderObject);
	glAttachShader(PBRShader, gFragmentShaderObject);

	// pre-linking binding to vertex attribute
	glBindAttribLocation(PBRShader, RMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(PBRShader, RMC_ATTRIBUTE_NORMAL, "vNormal");
	glBindAttribLocation(PBRShader, RMC_ATTRIBUTE_TEXTURE0, "vTexcoord");
	glBindAttribLocation(PBRShader, RMC_ATTRIBUTE_BONEIDS, "vBoneIDs");
	glBindAttribLocation(PBRShader, RMC_ATTRIBUTE_BONEWEIGHTS, "vBoneWeights");

	// link program
	if (!LinkProgram(PBRShader, "PBRShader")) return false;

	// post-linking retrieving uniform locations
	PBRUniforms = (PBRShaderUniforms*) malloc(sizeof(PBRShaderUniforms));
	PBRUniforms->mMatrixUniform = glGetUniformLocation(PBRShader, "u_modelMatrix");
	PBRUniforms->vMatrixUniform = glGetUniformLocation(PBRShader, "u_viewMatrix");
	PBRUniforms->pMatrixUniform = glGetUniformLocation(PBRShader, "u_projectionMatrix");

	PBRUniforms->boneMatrixUniform = glGetUniformLocation(PBRShader, "u_boneMatrix");

	PBRUniforms->lightPosUniform = glGetUniformLocation(PBRShader, "lightPosition");
	PBRUniforms->lightColUniform = glGetUniformLocation(PBRShader, "lightColor");
	PBRUniforms->cameraPosUniform = glGetUniformLocation(PBRShader, "cameraPos");

	PBRUniforms->alpha = glGetUniformLocation(PBRShader, "alpha");

	GLint albedoMapUniform = glGetUniformLocation(PBRShader, "albedoMap");
	GLint normalMapUniform = glGetUniformLocation(PBRShader, "normalMap");
	GLint metallicMapUniform = glGetUniformLocation(PBRShader, "metallicMap");
	GLint roughnessMapUniform = glGetUniformLocation(PBRShader, "roughnessMap");
	GLint aoMapUniform = glGetUniformLocation(PBRShader, "aoMap");

	// set constant uniforms
	glUseProgram(PBRShader);
	glUniform1i(albedoMapUniform, 0);
	glUniform1i(normalMapUniform, 1);
	glUniform1i(metallicMapUniform, 2);
	glUniform1i(roughnessMapUniform, 3);
	glUniform1i(aoMapUniform, 4);
	glUseProgram(0);

	LogD("PBR Shader compiled..");
	return true;
}

void UninitPBRShader()
{
	if (PBRShader)
	{
		DeleteProgram(PBRShader);
		PBRShader = 0;
		LogD("PBR Shader deleted..");
	}
}

PBRShaderUniforms *UsePBRShader()
{
	glUseProgram(PBRShader);
	return PBRUniforms;
}

