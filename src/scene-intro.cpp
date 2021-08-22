// headers
#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "scene-intro.h"

#include "material.h"
#include "primitives.h"
#include "framebuffer.h"
#include "model.h"
#include "letters.h"

// shaders
#include "PBRShader.h"
#include "TextureShader.h"
#include "BlurShader.h"
#include "BloomShader.h"

// effects
#include "rigidBody.h"
#include "camera.h"

// scene variable
Scene* SceneIntro = NULL;

namespace intro
{
	// scene state
	int gWidth, gHeight;
	mat4 projMatrix;

	Material* matPlastic = NULL;

	World world1;
	Marble marbles[20];
	Wall walls[10];
	ALuint audio[8];

	Framebuffer* fboMain = NULL;
	Framebuffer* fboPingpong[2] = { NULL, NULL };

	Model* sat = NULL;

	bool Init(void)
	{
		// matrix
		projMatrix = mat4::identity();

		matPlastic = loadMaterial("res\\materials\\plastic");

		audio[0] = LoadAudio("res\\audio\\01.wav");
		audio[1] = LoadAudio("res\\audio\\02.wav");
		audio[2] = LoadAudio("res\\audio\\03.wav");
		audio[3] = LoadAudio("res\\audio\\04.wav");
		audio[4] = LoadAudio("res\\audio\\05.wav");
		audio[5] = LoadAudio("res\\audio\\06.wav");
		audio[6] = LoadAudio("res\\audio\\07.wav");

		sat = LoadModel("res\\models\\saturn.obj", false);

		FramebufferParams params;
		params.width = 800;
		params.height = 600;
		params.nColors = 2;
		params.bDepth = false;

		fboPingpong[0] = CreateFramebuffer(&params);
		fboPingpong[1] = CreateFramebuffer(&params);

		params.bDepth = true;
		fboMain = CreateFramebuffer(&params);

		world1.cam = SceneIntro->Camera;
		return true;
	}

	void Uninit(void)
	{
		deleteMaterial(matPlastic);

		for (int i = 0; i < 7; i++)
		{
			UnloadAudio(audio[i]);
		}

		if (fboMain)
		{
			DeleteFramebuffer(fboMain);
		}

		if (fboPingpong[0])
		{
			DeleteFramebuffer(fboPingpong[0]);
		}

		if (fboPingpong[1])
		{
			DeleteFramebuffer(fboPingpong[1]);
		}

		// free the scene
		if (SceneIntro)
		{
			free(SceneIntro);
			SceneIntro = NULL;
		}
	}

	void Display(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain->fbo);

		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alListenerfv(AL_POSITION, normalize(SceneIntro->Camera->Position));

		projMatrix = vmath::perspective(45.0f + SceneIntro->Camera->Zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

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
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneIntro->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		glUniform3fv(u->cameraPosUniform, 1, SceneIntro->Camera->Position);
		glUniform1f(u->alpha, 1.0f);
		useMaterial(matPlastic);
		DrawCube();

		DrawWorld(world1); 

		// stop using OpenGL program object
		glUseProgram(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		BlurShaderUniforms* u1 = UseBlurShader();
		glUniform1i(u1->horizontal, 1);
		glUniform1i(u1->image, 0);

		unsigned int amount = 4;
		bool horizontal = true, first_iter = true;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboPingpong[horizontal]->fbo);
			glViewport(0, 0, gWidth, gHeight);
			glClear(GL_COLOR_BUFFER_BIT);
			glUniform1i(u1->horizontal, horizontal ? 1 : 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iter ? fboMain->colorTex[1] : fboPingpong[!horizontal]->colorTex[0]);

			DrawPlane();

			horizontal = !horizontal;
			if (first_iter) first_iter = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		BloomShaderUniforms* u2 = UseBloomShader();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboMain->colorTex[0]);
		glUniform1i(u2->tex1, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fboPingpong[!horizontal]->colorTex[0]);
		glUniform1i(u2->tex2, 1);
		DrawPlane();
	}

	bool Update(float delta)
	{
		// UpdateWorld(world1, 0.000002f * delta);
		return false;
	}

	void Resize(int width, int height)
	{
		const float dim = 50.0f;

		gWidth = width;
		gHeight = height;

		projMatrix = vmath::perspective(45.0f + SceneIntro->Camera->Zoom, (float)width / (float)height, 0.1f, 100.0f);
		ResizeFramebuffer(fboMain, width, height);
		ResizeFramebuffer(fboPingpong[0], width, height);
		ResizeFramebuffer(fboPingpong[1], width, height);
	}

	void Reset(void)
	{
		ResetWorld(world1);

		for (int i = 0; i < 13; i++)
		{
			marbles[i].Position = vec3((i - 3) * 2.5f, (i + 1) * 2.50f, 2.0f * (float)rand() / (float)RAND_MAX);
			marbles[i].Radius = 1.0f;
			marbles[i].Position = vec3(0.0f, (i + 1) * 2.50f, 0.0f);
			marbles[i].Velocity = vec3(2.0f * (float)rand() / (float)RAND_MAX, 0.0f, 0.0f);
			marbles[i].Mass = 10000.0f;
			marbles[i].mat = matPlastic;
			marbles[i].Audio = audio[i % 7];
			marbles[i].Angle = 0.0f;
			marbles[i].Axis = vec3();
			marbles[i].rotate = mat4::identity();
			marbles[i].xAngle = 0.0f;
			marbles[i].yAngle = 0.0f;
			marbles[i].zAngle = 0.0f;

			marbles[i].Color = vec3(100.0f, 100.0f, 0.0f);
			marbles[i].power = 0.01f;

			if (i % 4 == 0) marbles[i].Color = vec3(100.0f, 100.0f, 0.0f);
			if (i % 4 == 1) marbles[i].Color = vec3(0.0f, 0.0f, 100.0f);
			if (i % 4 == 2) marbles[i].Color = vec3(0.0f, 100.0f, 0.0f);
			if (i % 4 == 3) marbles[i].Color = vec3(100.0f, 0.0f, 0.0f);

		}

		marbles[0].mLetter = GetModel('A');
		marbles[1].mLetter = GetModel('S');
		marbles[2].mLetter = GetModel('T');
		marbles[3].mLetter = GetModel('R');
		marbles[4].mLetter = GetModel('O');

		marbles[5].mLetter = GetModel('M');
		marbles[6].mLetter = GetModel('E');
		marbles[7].mLetter = GetModel('D');
		marbles[8].mLetter = GetModel('I');

		marbles[9].mLetter = GetModel('C');
		marbles[10].mLetter = GetModel('O');
		marbles[11].mLetter = GetModel('M');
		marbles[12].mLetter = GetModel('P');

		AddMarble(world1, &marbles[0]);
		AddMarble(world1, &marbles[1]);
		AddMarble(world1, &marbles[2]);
		AddMarble(world1, &marbles[3]);
		AddMarble(world1, &marbles[4]);
		AddMarble(world1, &marbles[5]);
		AddMarble(world1, &marbles[6]);

		AddMarble(world1, &marbles[7]);
		AddMarble(world1, &marbles[8]);
		AddMarble(world1, &marbles[9]);
		AddMarble(world1, &marbles[10]);
		AddMarble(world1, &marbles[11]);
		AddMarble(world1, &marbles[12]);

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

		AddWall(world1, &walls[0]);
		AddWall(world1, &walls[1]);
		AddWall(world1, &walls[2]);
		AddWall(world1, &walls[3]);
		AddWall(world1, &walls[4]);
		//AddWall(world, &walls[5]);
	}
}

Scene* GetIntroScene()
{
	if (!SceneIntro)
	{
		SceneIntro = (Scene*)malloc(sizeof(Scene));

		strcpy_s(SceneIntro->Name, "IntroScene");

		SceneIntro->InitFunc = intro::Init;
		SceneIntro->UninitFunc = intro::Uninit;
		SceneIntro->ResetFunc = intro::Reset;

		SceneIntro->DisplayFunc = intro::Display;
		SceneIntro->UpdateFunc = intro::Update;
		SceneIntro->ResizeFunc = intro::Resize;

		SceneIntro->Camera = AddNewCamera(
			vec3(0.0f, 0.0f, -8.0f),
			vec3(0.0f, 0.0f, 1.0f),
			vec3(0.0f, 1.0f, 0.0f),
			90.0f, 0.0f);
	}

	return SceneIntro;
}
