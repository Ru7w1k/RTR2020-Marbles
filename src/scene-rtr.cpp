// headers
#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "scene-rtr.h"

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
#include "ParticleSystem.h"
#include "camera.h"

// scene variable
Scene *SceneRTR = NULL;

namespace rtr
{
	// scene state
	int gWidth, gHeight;
	mat4 projMatrix;

	Material *matPlastic = NULL;
	Material *matMarble = NULL;
	Material *matGround = NULL;

	World world;
	Marble marbles[200];
	Wall walls[10];
	ALuint audio[8];
	ALuint shoot;

	Framebuffer *fboMain = NULL;
	Framebuffer *fboPingpong[2] = {NULL, NULL};
	Framebuffer* fboPrevFrame = NULL;
	
	GLuint noiseTex;
	GLuint skyTex;

	float fadeV = 1.0f;
	int state = 0;
	Model* sat = NULL;

	bool Init(void)
	{
		// matrix
		projMatrix = mat4::identity();

		noiseTex = loadTexture("res\\textures\\noise2.png");
		skyTex = loadTexture("res\\textures\\sky.png");

		matPlastic = loadMaterial("res\\materials\\glass");
		matMarble = loadMaterial("res\\materials\\marble");
		matGround= loadMaterial("res\\materials\\wood");

		audio[0] = LoadAudio("res\\audio\\01.wav");
		audio[1] = LoadAudio("res\\audio\\02.wav");
		audio[2] = LoadAudio("res\\audio\\03.wav");
		audio[3] = LoadAudio("res\\audio\\04.wav");
		audio[4] = LoadAudio("res\\audio\\05.wav");
		audio[5] = LoadAudio("res\\audio\\06.wav");
		audio[6] = LoadAudio("res\\audio\\07.wav");

		shoot = LoadAudio("res\\audio\\shoot.wav");

		for (int i = 0; i < 7; i++)
		{
			alSourcef(audio[i], AL_GAIN, 6.0f);
		}
		alSourcef(shoot, AL_GAIN, 6.0f);

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

		params.bDepth = false;
		params.nColors = 1;
		fboPrevFrame = CreateFramebuffer(&params);

		world.cam = SceneRTR->Camera;
		return true;
	}

	void Uninit(void)
	{

		DeleteWorld(world);

		deleteMaterial(matPlastic);
		matPlastic = NULL;
		
		deleteMaterial(matGround);
		matGround = NULL;
		
		deleteMaterial(matMarble);
		matMarble = NULL;
		

		for(int i = 0; i < 7; i++)
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
		if (SceneRTR)
		{
			free(SceneRTR);
			SceneRTR = NULL;
		}
	}

	void Display(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain->fbo);
		
		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alListenerfv(AL_POSITION, SceneRTR->Camera->Position);
		alListenerfv(AL_ORIENTATION, new ALfloat[]{ SceneRTR->Camera->Position[0], SceneRTR->Camera->Position[1], SceneRTR->Camera->Position[2], 0.0f, 1.0f, 0.0f });

		projMatrix = vmath::perspective(45.0f + SceneRTR->Camera->Zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

		//TextureShaderUniforms *ut = UseTextureShader();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, skyTex);
		//glUniform1i(ut->samplerUniform, 0);
		//glUniform1f(ut->scaleUniform, 5.0f);

		//glCullFace(GL_FRONT);
		//glUniformMatrix4fv(ut->mvpMatrixUniform, 1, GL_FALSE, projMatrix * GetViewMatrixNoTranslate(SceneRTR->Camera) * scale(80.0f, 80.0f, 80.0f));
		//DrawSphere();
		//glUseProgram(0);
		//glCullFace(GL_BACK);


		// start using OpenGL program object
		PBRShaderUniforms* u = UsePBRShader();

		//declaration of matrices
		mat4 modelMatrix;

		// intialize above matrices to identity
		modelMatrix = mat4::identity();


		// send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneRTR->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		glUniform3fv(u->cameraPosUniform, 1, SceneRTR->Camera->Position);
		
		glUniform1f(u->alpha, 1.0f);

		//modelMatrix = scale(100.0f, 0.5f, 100.0f);
		//glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		//useMaterial(matGround);
		//DrawCube();

		modelMatrix = scale(15.0f, 0.5f, 15.0f);
		useMaterial(matPlastic);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		DrawCube();

		modelMatrix = translate(0.0f, 15.0f, 0.0f);
		modelMatrix *= scale(15.0f, 15.0f, 15.0f);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		DrawBox();

		glUseProgram(0);

		/*glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTex);*/

		DrawWorld(world);

		// stop using OpenGL program object
		glUseProgram(0);

		BlurShaderUniforms *u1 = UseBlurShader();
		glUniform1i(u1->horizontal, 1);
		glUniform1i(u1->image, 0);

		unsigned int amount = 4;
		bool horizontal = true, first_iter = true;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboPingpong[horizontal]->fbo);
			glViewport(0, 0, gWidth , gHeight);
			glClear(GL_COLOR_BUFFER_BIT);
			glUniform1i(u1->horizontal, horizontal?1:0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iter ? fboMain->colorTex[1] : fboPingpong[!horizontal]->colorTex[0]);

			DrawPlane();

			horizontal = !horizontal;
			if (first_iter) first_iter = false;
		}
		glUseProgram(0);

		// FINAL RENDER
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		glUseProgram(0);

		if (state == 0 || state == 3)
		{
			FadeShaderUniforms* u3 = UseFadeShader();
			glUniform1f(u3->fade, fadeV);
			DrawPlane();
			glUseProgram(0);
		}

	}

	bool Update(float delta)
	{
		static int i = 2;
		static int t = 0;
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
			if (t > 10)
			{
				if (i > 15)
				{
					state = 2;
				}
				else
				{
					AddMarble(world, &marbles[i]);
					marbles[i].power = 0.03f;
					world.Marbles[i]->Active = true;

					if (i % 2 == 0)
					{
						alSourcefv(shoot, AL_POSITION, -world.Marbles[0]->Position);
						PlayAudio(shoot);
					}
					else
					{
						alSourcefv(shoot, AL_POSITION, -world.Marbles[1]->Position);
						PlayAudio(shoot);
					}

					t = 0;
					i++;
				}
			}
			t++;
			
			UpdateWorld(world, 0.000002f * delta);
		}

		if (state == 2)
		{
			if (t > 40)
			{
				if (i > 99)
				{
					state = 2;
				}
				else
				{
					AddMarble(world, &marbles[i]);
					marbles[i].power = 0.03f;
					world.Marbles[i]->Active = true;

					if (i % 2 == 0)
					{
						alSourcefv(shoot, AL_POSITION, -world.Marbles[0]->Position);
						PlayAudio(shoot);
					}
					else
					{
						alSourcefv(shoot, AL_POSITION, -world.Marbles[1]->Position);
						PlayAudio(shoot);
					}

					t = 0;
					i++;
				}
			}
			t++;

			
			UpdateWorld(world, 0.000002f * delta);

			if (SceneRTR->Camera->Zoom <= -18.0f) state++;
		}

		SceneRTR->Camera->Yaw += 0.06f;
		SceneRTR->Camera->Pitch += 0.001f;
		SceneRTR->Camera->Zoom -= 0.003f;
		UpdateCameraVectors(SceneRTR->Camera);

		if (state == 3)
		{
			UpdateWorld(world, 0.000002f * delta);

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

		projMatrix = vmath::perspective(45.0f + SceneRTR->Camera->Zoom, (float)width / (float)height, 0.1f, 100.0f);
		ResizeFramebuffer(fboMain, width, height);
		ResizeFramebuffer(fboPingpong[0], width, height);
		ResizeFramebuffer(fboPingpong[1], width, height);
		ResizeFramebuffer(fboPrevFrame, width, height);
	}

	void Reset(void)
	{
		ResetWorld(world);

		fadeV = 1.0f;
		state = 0;
		world.ground = 0.62f;

		const char letters[] = { 'R', 'T', 'R', '2', '0', '2', '0', 'A', 'S', 'T', 'R', 'O', 'M', 'E', 'D', 'I', 'C', 'O', 'M', 'P' };
		const vec3 colors[] = {
			vec3(100.0f, 1.0f, 1.0f),
			vec3(1.0f, 100.0f, 1.0f),
			vec3(1.0f, 1.0f, 100.0f),
			vec3(100.0f, 100.0f, 1.0f),
			vec3(1.0f, 100.0f, 100.0f),
			vec3(10.0f, 10.0f, 100.0f),
			vec3(100.0f, 10.0f, 1.0f),
		};

		for (int i = 0; i < 100; i++)
		{
			marbles[i].Position = vec3(genRand(-8.0f, 8.0f), genRand(20.0f, 25.0f), genRand(-8.0f, 8.0f));
			marbles[i].Radius = 1.0f;
			marbles[i].Velocity = genVec3(-0.095f, 0.095f, -0.05f, -0.01f, -0.095f, 0.095f);
			marbles[i].Mass = 10000.0f;
			marbles[i].mat = matMarble;
			marbles[i].Audio = audio[i % 7];
			marbles[i].Angle = 0.0f;
			marbles[i].Axis = vec3();
			marbles[i].rotate = mat4::identity();
			marbles[i].xAngle = genRand(-3.14f, 3.14f);
			marbles[i].yAngle = genRand(-3.14f, 3.14f);
			marbles[i].zAngle = genRand(-3.14f, 3.14f);
			marbles[i].power = 0.001f;
			marbles[i].Active = false;

			marbles[i].Color = 0.2f * colors[i % _ARRAYSIZE(colors)];;
			//marbles[i].Color = 20.0f*GetRGBFromHSL(1.0f + ((i*4) % 360), 1.0f, 0.5f);
			//marbles[i].Color = 20.0f*GetRGBFromHSL(genRand(1.0f, 359.0f), 1.0f, 0.5f);
			marbles[i].mLetter = GetModel(letters[i % _ARRAYSIZE(letters)]);

			if (i < 100)
			{
				if (i % 2 == 0) marbles[i].Position = vec3(9.0f, 8.0f, -9.0f);
				else marbles[i].Position = vec3(-9.0f, 8.0f, 9.0f);

				marbles[i].Velocity = genVec3(0.010f, 0.015f, 0.20f, 0.25f, 0.010f, 0.015f);
				marbles[i].Active = false;
			}
		}

		marbles[0].Position = vec3(9.0f, 8.0f, -9.0f);
		marbles[0].Active = false;
		marbles[0].mLetter = NULL;
		AddMarble(world, &marbles[0]);

		marbles[1].Position = vec3(-9.0f, 8.0f, 9.0f);
		marbles[1].Active = false;
		marbles[1].mLetter = NULL;
		AddMarble(world, &marbles[1]);
		
		walls[0].Normal = normalize(vec3(0.0f, 1.0f, 0.0f));
		walls[0].D = -0.5f;

		walls[1].Normal = normalize(vec3(1.0f, 0.0f, 0.0f));
		walls[1].D = -15.0f;

		walls[2].Normal = normalize(vec3(-1.0f, 0.0f, 0.0f));
		walls[2].D = -15.0f;

		walls[3].Normal = normalize(vec3(0.0f, 0.0f, 1.0f));
		walls[3].D = -15.0f;

		walls[4].Normal = normalize(vec3(0.0f, 0.0f, -1.0f));
		walls[4].D = -15.0f;

		AddWall(world, &walls[0]);
		AddWall(world, &walls[1]);
		AddWall(world, &walls[2]);
		AddWall(world, &walls[3]);
		AddWall(world, &walls[4]);
	}
}

Scene *GetRTRScene()
{
	if (!SceneRTR)
	{
		SceneRTR = (Scene*)malloc(sizeof(Scene));

		strcpy_s(SceneRTR->Name, "RTRScene");

		SceneRTR->InitFunc   = rtr::Init;
		SceneRTR->UninitFunc = rtr::Uninit;
		SceneRTR->ResetFunc  = rtr::Reset;

		SceneRTR->DisplayFunc = rtr::Display;
		SceneRTR->UpdateFunc  = rtr::Update;
		SceneRTR->ResizeFunc  = rtr::Resize;

		// Position: vec3(24.915f, 22.933f, -24.915f)
		// Front : vec3(-0.623f, -0.473f, 0.623f)
		// Right : vec3(-0.707f, 0.000f, -0.707f)
		// Up : vec3(-0.335f, 0.881f, 0.335f)
		// Yaw : 135.000f
		// Pitch : -17.500f
		// Zoom :  -6.659f
		// Height : 4.000f

		SceneRTR->Camera = AddNewCamera(
			vec3(-0.623f, -0.473f, 0.623f),
			vec3(-0.623f, -0.473f, 0.623f),
			vec3(0.0f, 1.0f, 0.0f),
			135.0f, -17.50f,
			-8.65f, 5.0f);
	}

	return SceneRTR;
}
