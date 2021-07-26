// headers
#include "main.h"
#include "helper.h"
#include "scene-marbles.h"
#include "logger.h"

#include "rigidBody.h"

#include "material.h"
#include "primitives.h"

// shaders
#include "PBRShader.h"

// effects
#include "ParticleSystem.h"
#include "camera.h"

// scene variable
Scene *SceneMarbles = NULL;

namespace marbles
{
	// scene state
	int gWidth, gHeight;
	mat4 projMatrix;

	float angleTriangle = 0.0f;

	GLuint texParticle;
	ParticleSystem* ps = NULL;
	ParticleSystemParams params;

	Material *matWood = NULL;

	World world;
	Marble marbles[10];
	Wall walls[10];

	bool Init(void)
	{
		// matrix
		projMatrix = mat4::identity();

		// effects
		params.count = 100;
		params.lifespan = 100.0f;
		params.emitter = vec3(0.0f, 0.0f, 0.0f);

		float vel[] = { -0.2f, 0.2f, 0.5f, 1.5f, 0.1f, 0.2f };
		for (int i = 0; i < 6; i++)
			params.initVel[i] = vel[i];

		ps = newParticleSystem(&params);
		// texParticle = loadTexture("res\\textures\\part.png");
		texParticle = loadTexture("res\\textures\\particle.png");

		ps->tex = texParticle;
		ps->size = 32.0f;
		ps->color = vec4(1.0f, 0.5f, 0.1f, 1.0f);

		//matWood = loadMaterial("res\\materials\\wood");
		//matWood = loadMaterial("res\\materials\\rusted_iron");
		matWood = loadMaterial("res\\materials\\plastic");
				
		return true;
	}

	void Uninit(void)
	{
		if (ps)
		{
			deleteParticleSystem(ps);
		}

		// free the scene
		if (SceneMarbles)
		{
			free(SceneMarbles);
			SceneMarbles = NULL;
		}
	}

	void Display(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Particle system!
		/*ps->mvpMatrix = projMatrix * GetViewMatrix(SceneMarbles->Camera);
		drawParticleSystem(ps);*/
		projMatrix = vmath::perspective(45.0f + SceneMarbles->Camera->Zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

		// start using OpenGL program object
		PBRShaderUniforms* u = UsePBRShader();

		//declaration of matrices
		mat4 modelMatrix;

		// intialize above matrices to identity
		modelMatrix = mat4::identity();

		// transformations
		modelMatrix = scale(20.0f, 0.5f, 20.0f);

		// send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneMarbles->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		vec3 lightPos = vec3(0.0f, 10.0f, 0.0f);
		glUniform3fv(u->cameraPosUniform, 1, SceneMarbles->Camera->Position);

		useMaterial(matWood);
		DrawCube();

		/*glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, translate(0.0f, 6.0f, 0.0f) * rotate(angleTriangle, 0.0f, 1.0f, 0.0f));
		DrawSphere();

		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, translate(6.0f, 6.0f, 0.0f) * rotate(angleTriangle, 0.0f, 1.0f, 0.0f));
		DrawSphere();

		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, translate(6.0f, 6.0f, 6.0f) * rotate(angleTriangle, 0.0f, 1.0f, 0.0f));
		DrawSphere();*/

		DrawWorld(world);

		// stop using OpenGL program object
		glUseProgram(0);

	}

	bool Update(float delta)
	{
		angleTriangle += 0.000002f * delta;
		//if (angleTriangle > 90.0f) return true;

		//updateParticleSystem(ps);

		UpdateWorld(world, 0.000002f * delta);


		return false;
	}

	void Resize(int width, int height)
	{
		const float dim = 50.0f;
		
		gWidth = width;
		gHeight = height;

		if (width < height)
		{
			projMatrix = vmath::ortho(
				-dim, dim,
				-dim * ((float)height / (float)width), dim * ((float)height / (float)width),
				-dim, dim);
		}
		else
		{
			projMatrix = vmath::ortho(
				-dim * ((float)width / (float)height), dim * ((float)width / (float)height),
				-dim, dim,
				-dim, dim);
		}

		/*projMatrix = vmath::ortho(
			-dim, dim,
			-dim, dim,
			-dim, dim);*/

		projMatrix = vmath::perspective(45.0f + SceneMarbles->Camera->Zoom, (float)width / (float)height, 0.1f, 100.0f);

	}

	void Reset(void)
	{
		ResetWorld(world);

		marbles[0].Position = vec3(0.0f, 6.0f, 0.0f);
		marbles[0].Radius = 1.0f;
		marbles[0].Mass = 1.0f;
		marbles[0].Velocity = vec3(0.02f, 0.0f, 0.0f);
		marbles[0].Color = vec3(200.0f, 200.0f, 200.0f);

		marbles[1].Position = vec3(6.0f, 6.0f, 0.0f);
		marbles[1].Radius = 1.0f;
		marbles[1].Mass = 2.0f;
		marbles[1].Velocity = vec3(0.02f, 0.02f, 0.0f);
		marbles[1].Color = vec3(200.0f, 0.0f, 200.0f);

		marbles[2].Position = vec3(6.0f, 6.0f, 6.0f);
		marbles[2].Radius = 1.0f;
		marbles[2].Mass = 4.0f;
		marbles[2].Velocity = vec3(0.02f, 0.02f, 0.002f);
		marbles[2].Color = vec3(0.0f, 200.0f, 200.0f);

		marbles[3].Position = vec3(6.0f, 7.0f, 6.0f);
		marbles[3].Radius = 1.0f;
		marbles[3].Mass = 10.0f;
		marbles[3].Velocity = vec3(0.02f, 0.02f, 0.002f);
		marbles[3].Color = vec3(200.0f, 200.0f, 0.0f);

		marbles[4].Position = vec3(6.0f, 4.0f, 6.0f);
		marbles[4].Radius = 1.0f;
		marbles[4].Mass = 6.0f;
		marbles[4].Velocity = vec3(0.02f, 0.02f, 0.002f);
		marbles[4].Color = vec3(200.0f, 0.0f, 0.0f);

		marbles[5].Position = vec3(4.0f, 6.0f, 6.0f);
		marbles[5].Radius = 1.0f;
		marbles[5].Mass = 5.0f;
		marbles[5].Velocity = vec3(0.02f, 0.02f, 0.002f);
		marbles[5].Color = vec3(0.0f, 200.0f, 0.0f);

		AddMarble(world, &marbles[0]);
		AddMarble(world, &marbles[1]);
		AddMarble(world, &marbles[2]);
		AddMarble(world, &marbles[3]);
		AddMarble(world, &marbles[4]);
		AddMarble(world, &marbles[5]);

		walls[0].Normal = vec3(0.0f, 1.0f, 0.0f);
		walls[0].D = -0.5f;

		walls[1].Normal = normalize(vec3(0.0f, 1.0f, 1.0f));
		walls[1].D = -2.0f;

		walls[2].Normal = normalize(vec3(0.0f, 1.0f, -1.0f));
		walls[2].D = -2.0f;

		AddWall(world, &walls[0]);
		//AddWall(world, &walls[1]);
		//AddWall(world, &walls[2]);
	}
}

Scene *GetMarblesScene()
{
	if (!SceneMarbles)
	{
		SceneMarbles = (Scene*)malloc(sizeof(Scene));

		strcpy_s(SceneMarbles->Name, "MarblesScene");

		SceneMarbles->InitFunc   = marbles::Init;
		SceneMarbles->UninitFunc = marbles::Uninit;
		SceneMarbles->ResetFunc  = marbles::Reset;

		SceneMarbles->DisplayFunc = marbles::Display;
		SceneMarbles->UpdateFunc  = marbles::Update;
		SceneMarbles->ResizeFunc  = marbles::Resize;

		SceneMarbles->Camera = AddNewCamera(
			vec3(0.0f, 0.0f, -8.0f), 
			vec3(0.0f, 0.0f, 1.0f), 
			vec3(0.0f, 1.0f, 0.0f), 
			90.0f, 0.0f);
	}

	return SceneMarbles;
}
