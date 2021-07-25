// headers
#include "main.h"
#include "helper.h"
#include "logger.h"

#include "ParticleSystem.h"

// shaders
#include "ParticleShader.h"

ParticleSystem* newParticleSystem(ParticleSystemParams *params)
{
	ParticleSystem* ps = (ParticleSystem*)malloc(sizeof(ParticleSystem));

	ps->count    = params->count;
	ps->emitter  = params->emitter;
	ps->lifespan = params->lifespan;
	ps->color    = params->color;
	
	for (int i = 0; i < 6; i++)
		ps->initVel[i] = params->initVel[i];

	ps->pos = (vec3*)calloc(ps->count, sizeof(vec3));
	ps->vel = (vec3*)calloc(ps->count, sizeof(vec3));
	ps->acc = (vec3*)calloc(ps->count, sizeof(vec3));

	ps->life = (float*)calloc(ps->count, sizeof(float));

	ps->mvpMatrix = mat4::identity();
	ps->tex = -1;

	// rendering related stuff
	glGenVertexArrays(1, &ps->vao);
	glBindVertexArray(ps->vao);

	glGenBuffers(1, &ps->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ps->vbo);
	glBufferData(GL_ARRAY_BUFFER, ps->count * sizeof(vec3), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(RMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(RMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	
	return ps;
}

void drawParticleSystem(ParticleSystem* ps)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_POINT_SPRITE);
	glPointSize(ps->size);
	
	glBindVertexArray(ps->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ps->vbo);
	glBufferData(GL_ARRAY_BUFFER, ps->count * sizeof(vec3), ps->pos, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	ParticleShaderUniforms* u = UseParticleShader();
	glUniformMatrix4fv(u->mvpMatrixUniform, 1, GL_FALSE, ps->mvpMatrix);
	glUniform1fv(u->lifeUniform, ps->count, ps->life);
	glUniform1f(u->lifespanUniform, 1.0f / ps->lifespan);
	glUniform4fv(u->colorUniform, 1, ps->color);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ps->tex);
	glUniform1i(u->texSamplerUniform, 0);

	glDrawArrays(GL_POINTS, 0, ps->count);
	glBindVertexArray(0);

	glPointSize(1.0f);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void updateParticleSystem(ParticleSystem* ps)
{
	for (int i = 0; i < ps->count; i++)
	{
		// advance particle with current data
		ps->vel[i]  += 0.01f * ps->acc[i];
		ps->pos[i]  += 0.01f * ps->vel[i];
		ps->life[i] -= 1.5f;
		
		if (ps->life[i] < 0.0f)
		{
			// particle is dead
			// fill index with new particle
			ps->pos[i] = ps->emitter;
			ps->vel[i] = genVec3(ps->initVel);
			ps->acc[i] = vec3(-0.01f);

			ps->life[i] = GEN_RAND * ps->lifespan;
		}
	}

	// set acc to 0 for all particles 
	memset(ps->acc, 0, ps->count * sizeof(vec3));
}

void deleteParticleSystem(ParticleSystem* ps)
{
	if (!ps) return;

	free(ps->pos);
	free(ps->vel);
	free(ps->acc);
	free(ps->life);
	free(ps);
}

