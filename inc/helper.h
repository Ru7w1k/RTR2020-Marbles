#pragma once

// helper functions
void InitClock();
float GetCurrentTimeMS();
float GetTimeDeltaMS();

// shader helpers
bool CompileShader(GLuint&, const char*, GLenum, const char*);
bool LinkProgram(GLuint, const char*);
void DeleteProgram(GLuint);

// random helpers
#define GEN_RAND (((float)rand())/(float)RAND_MAX)
float genRand(float, float);
vec3  genVec3(float, float, float, float, float, float);
vec3  genVec3(float[6]);

// texture helpers
GLuint loadTexture(const char* filename);

