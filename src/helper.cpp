#include <Windows.h>
#include "main.h"
#include "helper.h"
#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static LARGE_INTEGER prevTime;
static LARGE_INTEGER curTime;
static LARGE_INTEGER temp;

static double freq = 0.0f;

// Initialize the clock
void InitClock()
{
    // get the frequency from OS
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&temp));
    freq = (static_cast<double>(temp.QuadPart)) / 1000.0;

    QueryPerformanceCounter(&prevTime);
    curTime = prevTime;
}

// Get current time in microseconds
float GetCurrentTimeMS()
{
    QueryPerformanceCounter(&temp);
    temp.QuadPart *= 10000;
    temp.QuadPart /= (LONGLONG)freq;

    return (float)temp.QuadPart;
}

// Get time different between now and previous call
float GetTimeDeltaMS()
{
    QueryPerformanceCounter(&temp);
    prevTime = curTime;
    curTime = temp;

    temp.QuadPart = curTime.QuadPart - prevTime.QuadPart;
    temp.QuadPart *= 10000;
    temp.QuadPart /= (LONGLONG)freq;

    return (float)temp.QuadPart;
}

//////////////////////////////////////////////////////////////////////
// shader related helpers 

bool CompileShader(GLuint &shader, const char *source, GLenum type, const char *name)
{
	// variables
	int iShaderCompileStatus = 0;
	int iInfoLogLength = 0;
	char *szInfoLog = NULL;

	// create shader and attach source code
    shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, NULL);

	// compile shader and check errors
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(shader, GL_INFO_LOG_LENGTH, &written, szInfoLog);
				LogE("%s Shader Compiler Info Log: \n%s", name, szInfoLog);
				free(szInfoLog);
				return false;
			}
		}
	}

	return true;
}

bool LinkProgram(GLuint program, const char* name)
{
	// variables
	GLint iProgramLinkStatus = 0;
	int iInfoLogLength = 0;
	char* szInfoLog = NULL;

	// link program and check errors
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(program, GL_INFO_LOG_LENGTH, &written, szInfoLog);
				LogE(("%s Program Linking Info Log: \n%s"), name, szInfoLog);
				free(szInfoLog);
				return false;
			}
		}
	}
	return true;
}

void DeleteProgram(GLuint program)
{
	// destroy shader programs
	if (program)
	{
		GLsizei shaderCount;
		GLsizei i;

		glUseProgram(program);
		glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaderCount);

		GLuint* pShaders = (GLuint*)malloc(shaderCount * sizeof(GLuint));
		if (pShaders)
		{
			glGetAttachedShaders(program, shaderCount, &shaderCount, pShaders);

			for (i = 0; i < shaderCount; i++)
			{
				// detach shader
				glDetachShader(program, pShaders[i]);

				// delete shader
				glDeleteShader(pShaders[i]);
				pShaders[i] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(program);
		glUseProgram(0);
	}
}

//////////////////////////////////////////////////////////////////////
// random related helpers

float genRand(float min, float max)
{
	return min + (GEN_RAND * (max - min));
}

vec3 genVec3(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
	return vec3(
		minX + (GEN_RAND * (maxX - minX)),
		minY + (GEN_RAND * (maxY - minY)),
		minZ + (GEN_RAND * (maxZ - minZ))
	);
}

vec3 genVec3(float v[6])
{
	return vec3(
		v[0] + (GEN_RAND * (v[1] - v[0])),
		v[2] + (GEN_RAND * (v[3] - v[2])),
		v[4] + (GEN_RAND * (v[5] - v[4]))
	);
}

//////////////////////////////////////////////////////////////////////
// texture helpers

GLuint loadTexture(const char* filename)
{
	GLuint textureID = 0;

	int width, height, nrComponents;
	unsigned char* image = stbi_load(filename, &width, &height, &nrComponents, 0);
	if (image)
	{
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(image);
		LogD("texture loaded at %s..", filename);
	}
	else
	{
		LogE("failed to load texture at %s..", filename);
	}

	return textureID;
}

//////////////////////////////////////////////////////////////////////
// Convert color from HSL format to RGB format
vec3 GetRGBFromHSL(float H, float S, float L)
{
	float R, G, B;

	// chroma
	float C = (1.0f - fabsf(2.0f * L - 1.0f)) * S;
	float _H = H / 60.0f;
	float X = C * (1.0f - fabsf(fmodf(_H, 2.0f) - 1.0f));

	INT Hdash = (INT)ceilf(_H);
	switch (Hdash)
	{
	case 1:
		R = C;
		G = X;
		B = 0.0f;
		break;

	case 2:
		R = X;
		G = C;
		B = 0.0f;
		break;

	case 3:
		R = 0;
		G = C;
		B = X;
		break;

	case 4:
		R = 0;
		G = X;
		B = C;
		break;

	case 5:
		R = X;
		G = 0;
		B = C;
		break;

	case 6:
		R = C;
		G = 0;
		B = X;
		break;

	default:
		R = 0.0f;
		G = 0.0f;
		B = 0.0f;
		break;
	}

	float m = L - (C / 2.0f);
	R += m;
	G += m;
	B += m;

	LogD("%5.2f %5.2f %5.2f -> %5.2f %5.2f %5.2f", H, S, L, R, G, B);

	return vec3(R, G, B);
}

//////////////////////////////////////////////////////////////////////
