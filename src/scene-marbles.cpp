// headers
#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "scene-marbles.h"

#include "material.h"
#include "primitives.h"
#include "framebuffer.h"
#include "model.h"

// shaders
#include "PBRShader.h"
#include "TextureShader.h"
#include "BlurShader.h"

// effects
#include "rigidBody.h"
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

	Material *matPlastic = NULL;
	Material* mat[5];

	World world;
	Marble marbles[10];
	Wall walls[10];
	ALuint audio[8];

	Framebuffer *fboMain = NULL;
	Model* R = NULL;

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

		//matPlastic = loadMaterial("res\\materials\\wood");
		//matWood = loadMaterial("res\\materials\\rusted_iron");
		matPlastic = loadMaterial("res\\materials\\plastic");

		mat[0] = loadMaterial("res\\materials\\plastic");
		/*mat[1] = loadMaterial("res\\materials\\wood");
		mat[2] = loadMaterial("res\\materials\\gold");
		mat[3] = loadMaterial("res\\materials\\rusted_iron");
		mat[4] = loadMaterial("res\\materials\\grass");*/

		audio[0] = LoadAudio("res\\audio\\01.wav");
		audio[1] = LoadAudio("res\\audio\\02.wav");
		audio[2] = LoadAudio("res\\audio\\03.wav");
		audio[3] = LoadAudio("res\\audio\\04.wav");
		audio[4] = LoadAudio("res\\audio\\05.wav");
		audio[5] = LoadAudio("res\\audio\\06.wav");
		audio[6] = LoadAudio("res\\audio\\07.wav");

		R = LoadModel("res\\models\\R.obj", false);

		FramebufferParams params;
		params.width = 800;
		params.height = 600;
		params.nColors = 2;

		fboMain = CreateFramebuffer(&params);

		world.cam = SceneMarbles->Camera;

		return true;
	}

	void Uninit(void)
	{
		for(int i = 0; i < 5; i++)
		{
			deleteMaterial(mat[i]);
		}

		for(int i = 0; i < 7; i++)
		{
			UnloadAudio(audio[i]);
		}

		if (ps)
		{
			deleteParticleSystem(ps);
		}

		if (fboMain)
		{
			DeleteFramebuffer(fboMain);
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
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain->fbo);
		
		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alListenerfv(AL_POSITION, normalize(SceneMarbles->Camera->Position));

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
		modelMatrix = scale(10.0f, 0.5f, 10.0f);

		// send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneMarbles->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		vec3 lightPos = vec3(0.0f, 10.0f, 0.0f);
		glUniform3fv(u->cameraPosUniform, 1, SceneMarbles->Camera->Position);
		
		glUniform1f(u->alpha, 1.0f);
		useMaterial(matPlastic);
		DrawCube();

		modelMatrix = translate(0.0f, 2.0f, 0.0f);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		DrawModel(R);

		DrawWorld(world);

		// stop using OpenGL program object
		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		BlurShaderUniforms *u1 = UseBlurShader();
		glUniform1i(u1->horizontal, 1);
		glUniform1i(u1->image, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain->colorTex[1]);
		DrawPlane();

		//DrawFramebuffer(fboMain, 0);

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
		ResizeFramebuffer(fboMain, width, height);

	}

	void Reset(void)
	{
		ResetWorld(world);

		marbles[0].Position = vec3(0.0f, 6.0f, 0.0f);
		marbles[0].Radius = 1.0f;
		marbles[0].Mass = 1.0f;
		marbles[0].Velocity = vec3(0.04f, 0.0f, 0.0f);
		marbles[0].Color = vec3(200.0f, 200.0f, 200.0f);

		marbles[1].Position = vec3(6.0f, 6.0f, 0.0f);
		marbles[1].Radius = 1.0f;
		marbles[1].Mass = 2.0f;
		marbles[1].Velocity = vec3(0.04f, 0.04f, 0.0f);
		marbles[1].Color = vec3(200.0f, 0.0f, 200.0f);

		marbles[2].Position = vec3(6.0f, 6.0f, 6.0f);
		marbles[2].Radius = 1.0f;
		marbles[2].Mass = 4.0f;
		marbles[2].Velocity = vec3(0.04f, 0.04f, 0.04f);
		marbles[2].Color = vec3(0.0f, 200.0f, 200.0f);

		marbles[3].Position = vec3(6.0f, 7.0f, 6.0f);
		marbles[3].Radius = 1.0f;
		marbles[3].Mass = 10.0f;
		marbles[3].Velocity = vec3(0.04f, 0.02f, 0.002f);
		marbles[3].Color = vec3(200.0f, 200.0f, 0.0f);

		marbles[4].Position = vec3(6.0f, 4.0f, 6.0f);
		marbles[4].Radius = 1.0f;
		marbles[4].Mass = 6.0f;
		marbles[4].Velocity = vec3(0.02f, 0.04f, 0.002f);
		marbles[4].Color = vec3(200.0f, 0.0f, 0.0f);

		marbles[5].Position = vec3(4.0f, 6.0f, 6.0f);
		marbles[5].Radius = 1.0f;
		marbles[5].Mass = 5.0f;
		marbles[5].Velocity = vec3(0.02f, 0.02f, 0.002f);
		marbles[5].Color = vec3(0.0f, 200.0f, 0.0f);

		marbles[6].Position = vec3(4.0f, 6.0f, 3.0f);
		marbles[6].Radius = 1.0f;
		marbles[6].Mass = 3.0f;
		marbles[6].Velocity = vec3(0.02f, 0.02f, 0.002f);
		marbles[6].Color = vec3(0.0f, 200.0f, 200.0f);

		marbles[7].Position = vec3(4.0f, 12.0f, 6.0f);
		marbles[7].Radius = 1.0f;
		marbles[7].Mass = 15.0f;
		marbles[7].Velocity = vec3(0.02f, 0.32f, 0.012f);
		marbles[7].Color = vec3(100.0f, 200.0f, 0.0f);

		marbles[8].Position = vec3(2.0f, 15.0f, 4.0f);
		marbles[8].Radius = 1.0f;
		marbles[8].Mass = 1.0f;
		marbles[8].Velocity = vec3(0.04f, 0.02f, 0.042f);
		marbles[8].Color = vec3(0.0f, 200.0f, 100.0f);

		for (int i = 0; i < 9; i++)
		{
			marbles[i].Position = vec3((i-3)*2.5f, (i+1) * 5.50f, 2.0f * (float)rand() / (float)RAND_MAX);
			marbles[i].Velocity = vec3(0.1f, 0.0f, 0.0f);
			marbles[i].Mass = 10.0f;
			marbles[i].mat = mat[0];
			marbles[i].Audio = audio[i % 7];
			marbles[i].Angle = 0.0f;
			marbles[i].Axis = vec3();
			marbles[i].mLetter = R;
			marbles[i].rotate = mat4::identity();
			marbles[i].xAngle = 0.0f;
			marbles[i].yAngle = 0.0f;
			marbles[i].zAngle = 0.0f;
		}

		AddMarble(world, &marbles[0]);
		AddMarble(world, &marbles[1]);
		AddMarble(world, &marbles[2]);
		AddMarble(world, &marbles[3]);
		AddMarble(world, &marbles[4]);
		AddMarble(world, &marbles[5]);
		AddMarble(world, &marbles[6]);

		//AddMarble(world, &marbles[7]);
		//AddMarble(world, &marbles[8]);

		walls[0].Normal = normalize(vec3(0.0f, 1.0f, 0.0f));
		walls[0].D = -0.5f;

		walls[1].Normal = vec3(1.0f, 0.0f, 0.0f);
		walls[1].D = -15.0f;

		walls[2].Normal = vec3(-1.0f, 0.0f, 0.0f);
		walls[2].D = -15.0f;

		walls[3].Normal = vec3(0.0f, 0.0f, 1.0f);
		walls[3].D = -15.0f;

		walls[4].Normal = vec3(0.0f, 0.0f, -1.0f);
		walls[4].D = -15.0f;

		walls[5].Normal = vec3(0.0f, 1.0f, 0.0f);
		walls[5].D = -1.0f;
		walls[5].MinPoint = vec3(-2.0f, 1.0f, -2.0f);
		walls[5].MaxPoint = vec3(2.0f, 1.0f, 2.0f);

		AddWall(world, &walls[0]);
		AddWall(world, &walls[1]);
		AddWall(world, &walls[2]);
		AddWall(world, &walls[3]);
		AddWall(world, &walls[4]);
		//AddWall(world, &walls[5]);
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
