// headers
#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "scene-domain.h""

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
#include "FadeShader.h"

// effects
#include "rigidBody.h"
#include "camera.h"

// scene variable
Scene* SceneDomain = NULL;

namespace domain
{
	// scene state
	int gWidth, gHeight;
	mat4 projMatrix;

	Material* matGlass = NULL;
	Material* matMarble = NULL;

	World world;
	Marble marbles[20];
	ALuint audio[8];
	Wall walls[2];

	Framebuffer* fboMain = NULL;
	Framebuffer* fboPingpong[2] = { NULL, NULL };

	Model* sat = NULL;

	float fadeV = 1.0f;
	int state = 0;

	bool Init(void)
	{
		// matrix
		projMatrix = mat4::identity();

		matGlass = loadMaterial("res\\materials\\glass");
		matMarble = loadMaterial("res\\materials\\marble");

		audio[0] = LoadAudio("res\\audio\\01.wav");
		audio[1] = LoadAudio("res\\audio\\02.wav");
		audio[2] = LoadAudio("res\\audio\\03.wav");
		audio[3] = LoadAudio("res\\audio\\04.wav");
		audio[4] = LoadAudio("res\\audio\\05.wav");
		audio[5] = LoadAudio("res\\audio\\06.wav");
		audio[6] = LoadAudio("res\\audio\\07.wav");

		sat = LoadModel("res\\models\\saturn1.obj", false);

		FramebufferParams params;
		params.width = 800;
		params.height = 600;
		params.nColors = 2;
		params.bDepth = false;

		fboPingpong[0] = CreateFramebuffer(&params);
		fboPingpong[1] = CreateFramebuffer(&params);

		params.bDepth = true;
		fboMain = CreateFramebuffer(&params);

		world.cam = SceneDomain->Camera;
		return true;
	}

	void Uninit(void)
	{
		deleteMaterial(matGlass);

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
		if (SceneDomain)
		{
			free(SceneDomain);
			SceneDomain = NULL;
		}
	}

	void Display(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain->fbo);

		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alListenerfv(AL_POSITION, normalize(SceneDomain->Camera->Position));

		projMatrix = vmath::perspective(45.0f + SceneDomain->Camera->Zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

		// start using OpenGL program object
		PBRShaderUniforms* u = UsePBRShader();

		/*glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTex);*/
		DrawWorld(world); 

		// send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneDomain->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		glUniform3fv(u->cameraPosUniform, 1, SceneDomain->Camera->Position);
		glUniform1f(u->alpha, 1.0f);
		useMaterial(matGlass);

		mat4 modelMatrix = mat4::identity();
		modelMatrix = scale(30.0f, 0.5f, 30.0f);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		DrawCube();

		//modelMatrix = translate(0.0f, 0.0f, 15.0f);
		//modelMatrix *= scale(20.0f, 20.0f, 0.5f);
		//glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		//DrawCube();

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

		if (state != 1)
		{
			FadeShaderUniforms* u3 = UseFadeShader();
			glUniform1f(u3->fade, fadeV);
			DrawPlane();
		}
	}

	bool Update(float d)
	{
		static int i = 0;
		static int t = 0;
		static int delta = 12;
		static int n = 0;

		if (state == 0)
		{
			fadeV -= 0.01f;
			if (fadeV <= 0.0f)
			{
				fadeV = 0.0f;
				state = 1;
			}
		}

		if (state == 1)
		{
			if (t > delta)
			{
				if (i > 10)
				{
					i = 0;
					t = 0;
					delta = 60;
					n++;
					if (n == 3) state = 2;
				}
				else
				{
					alSourcefv(world.Marbles[i]->Audio, AL_POSITION, -world.Marbles[i]->Position);
					PlayAudio(world.Marbles[i]->Audio);
					world.Marbles[i]->power = 0.12f;
					t = 0;
					i++;

					if (i == 6) delta = 25;
					else delta = 10;
				}
			}
			t++;
		}

		if (state == 2)
		{
			fadeV += 0.01f;
			if (fadeV >= 1.0f)
			{
				fadeV = 1.0f;
				return true;
			}
		}

		return false;
	}

	void Resize(int width, int height)
	{
		const float dim = 50.0f;

		gWidth = width;
		gHeight = height;

		projMatrix = vmath::perspective(45.0f + SceneDomain->Camera->Zoom, (float)width / (float)height, 0.1f, 100.0f);
		ResizeFramebuffer(fboMain, width, height);
		ResizeFramebuffer(fboPingpong[0], width, height);
		ResizeFramebuffer(fboPingpong[1], width, height);
	}

	void Reset(void)
	{
		ResetWorld(world);

		fadeV = 1.0f;
		state = 0;

		for (int i = 0; i < 14; i++)
		{
			marbles[i].Radius = 1.0f;
			marbles[i].Velocity = vec3(0.0f, 0.0f, 0.0f);
			marbles[i].Mass = 10000.0f;
			marbles[i].mat = matMarble;
			marbles[i].Audio = audio[i % 7];
			marbles[i].Angle = 0.0f;
			marbles[i].Axis = vec3();
			marbles[i].rotate = mat4::identity();
			marbles[i].xAngle = radians(-90.0f);
			marbles[i].yAngle = 0.0;
			marbles[i].zAngle = 0.0f;
			marbles[i].power = 0.01f;
			marbles[i].Active = false;

			if (i > 5) 
			{
				// G R O U P
				marbles[i].Color = vec3(100.0f, 100.0f, 1.0f);
				marbles[i].Position = vec3(15.0f - (i*2.2f), 1.50f, -10.0f);
				marbles[i].Audio = audio[(i-4)];
			}
			else 
			{
				// D O M A I N
				marbles[i].Color = vec3(1.0f, 100.0f, 1.0f);
				marbles[i].Position = vec3(12.0f - (i*2.2f), 1.50f, -2.0f);
				marbles[i].Audio = audio[(i-0)];
			}
		}

		marbles[0].mLetter = GetModel('D');
		marbles[1].mLetter = GetModel('O');
		marbles[2].mLetter = GetModel('M');
		marbles[3].mLetter = GetModel('A');
		marbles[4].mLetter = GetModel('I');
		marbles[5].mLetter = GetModel('N');

		marbles[6].mLetter = GetModel('G');
		marbles[7].mLetter = GetModel('R');
		marbles[8].mLetter = GetModel('O');
		marbles[9].mLetter = GetModel('U');
		marbles[10].mLetter = GetModel('P');


		AddMarble(world, &marbles[0]);
		AddMarble(world, &marbles[1]);
		AddMarble(world, &marbles[2]);
		AddMarble(world, &marbles[3]);
		AddMarble(world, &marbles[4]);
		AddMarble(world, &marbles[5]);

		AddMarble(world, &marbles[6]);
		AddMarble(world, &marbles[7]);
		AddMarble(world, &marbles[8]);
		AddMarble(world, &marbles[9]);
		AddMarble(world, &marbles[10]);

		//AddMarble(world, &marbles[11]);
		//AddMarble(world, &marbles[12]);

		marbles[13].mLetter = sat;
		marbles[13].Color = vec3(150.0f, 100.0f, 0.0f);
		marbles[13].Position = vec3(8.0f, 1.50f, -20.0f);
		marbles[13].Radius = 1.5f;

		//AddMarble(world, &marbles[13]);

		walls[0].Normal = normalize(vec3(0.0f, 1.0f, 0.0f));
		walls[0].D = -0.5f;
		AddWall(world, &walls[0]);
	}
}

Scene* GetDomainScene()
{
	if (!SceneDomain)
	{
		SceneDomain = (Scene*)malloc(sizeof(Scene));

		strcpy_s(SceneDomain->Name, "DomainScene");

		SceneDomain->InitFunc = domain::Init;
		SceneDomain->UninitFunc = domain::Uninit;
		SceneDomain->ResetFunc = domain::Reset;

		SceneDomain->DisplayFunc = domain::Display;
		SceneDomain->UpdateFunc = domain::Update;
		SceneDomain->ResizeFunc = domain::Resize;
		
		// Position: 19.085726 6.047958 - 34.788692
		// Front : -0.477143 - 0.126199 0.869717
		// Right : -0.876727 0.000000 - 0.480989
		// Up : -0.060700 0.992005 0.110642
		// Yaw : 118.750000
		// Pitch : -7.250000
		// Zoom : -18.000000
		// Height : 1.000000 

		SceneDomain->Camera = AddNewCamera(
			vec3(19.08f, 6.04f, - 34.78f),
			vec3(-0.47f, - 0.12f, 0.86f),
			vec3(0.0f, 1.0f, 0.0f),
			118.75f, -7.25f,
			-18.0f, 1.0f);
	}

	return SceneDomain;
}
