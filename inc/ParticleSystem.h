#pragma once
#include "main.h"

typedef struct _particleSystemParams
{
	int   count;
	vec3  emitter;
	vec4  color;
	float lifespan;
	float initVel[6];

} ParticleSystemParams;

typedef struct _particleSystem 
{
	GLuint vao;
	GLuint vbo;

	vec3  *pos;
	vec3  *vel;
	vec3  *acc;
	float *life;

	// setup parameters
	int   count;
	vec3  emitter;
	float lifespan;
	float size;
	vec4  color;
	float initVel[6];

	mat4   mvpMatrix;
	GLuint tex;

} ParticleSystem;


ParticleSystem* newParticleSystem(ParticleSystemParams* params);
void drawParticleSystem(ParticleSystem* ps);
void updateParticleSystem(ParticleSystem* ps);
void deleteParticleSystem(ParticleSystem* ps);

