// headers
#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "scene-slope.h"

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
Scene *SceneSlope = NULL;

namespace slope
{
	// scene state
	int gWidth, gHeight;
	mat4 projMatrix;

	Material *matMarble = NULL;
	Material *matGround = NULL;

	World world;
	Marble marbles[200];
	int *notesTimes = NULL;
	int notesCount = 0;
	Wall walls[10];
	ALuint audio[8];
	ALuint shoot;

	Framebuffer *fboMain = NULL;
	Framebuffer *fboPingpong[2] = {NULL, NULL};
	Framebuffer* fboPrevFrame = NULL;
	
	float fadeV = 1.0f;
	int state = 0;
	Model* n1 = NULL;
	Model* n2 = NULL;
	Model* n3 = NULL;

	bool Init(void)
	{
		// matrix
		projMatrix = mat4::identity();

		matMarble = loadMaterial("res\\materials\\marble");
		matGround= loadMaterial("res\\materials\\piano");

		audio[0] = LoadAudio("res\\audio\\01.wav");
		audio[1] = LoadAudio("res\\audio\\02.wav");
		audio[2] = LoadAudio("res\\audio\\03.wav");
		audio[3] = LoadAudio("res\\audio\\04.wav");
		audio[4] = LoadAudio("res\\audio\\05.wav");
		audio[5] = LoadAudio("res\\audio\\06.wav");
		audio[6] = LoadAudio("res\\audio\\07.wav");

		for (int i = 0; i < 7; i++)
		{
			alSourcef(audio[i], AL_GAIN, 6.0f);
		}

		shoot = LoadAudio("res\\audio\\shoot.wav");

		n1 = LoadModel("res\\models\\n1.obj", false);
		n2 = LoadModel("res\\models\\n2.obj", false);
		n3 = LoadModel("res\\models\\n3.obj", false);

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

		world.cam = SceneSlope->Camera;
		return true;
	}

	void Uninit(void)
	{

		DeleteWorld(world);
		
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
		if (SceneSlope)
		{
			free(SceneSlope);
			SceneSlope = NULL;
		}
	}

	void Display(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboMain->fbo);
		
		glViewport(0, 0, gWidth, gHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alListenerfv(AL_POSITION, SceneSlope->Camera->Position);
		alListenerfv(AL_ORIENTATION, new ALfloat[]{ SceneSlope->Camera->Position[0], SceneSlope->Camera->Position[1], SceneSlope->Camera->Position[2], 0.0f, 1.0f, 0.0f });

		projMatrix = vmath::perspective(45.0f + SceneSlope->Camera->Zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

		// start using OpenGL program object
		PBRShaderUniforms* u = UsePBRShader();

		//declaration of matrices
		mat4 modelMatrix;

		// intialize above matrices to identity
		modelMatrix = mat4::identity();


		// send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneSlope->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		glUniform3fv(u->cameraPosUniform, 1, SceneSlope->Camera->Position);
		
		glUniform1f(u->alpha, 1.0f);

		modelMatrix = rotate(-28.0f, 0.0f, 0.0f, 1.0f);
		modelMatrix *= scale(25.0f, 0.5f, 25.0f);
		useMaterial(matGround);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		DrawCube();
		
		glUseProgram(0);

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
		static int i = 7;
		static int t = 0;
		static int n = 10;

		//  ---- - camera--------------------
		//  Position: vec3(27.899f, 18.257f, -27.450f)
		//  Front : vec3(-0.697f, -0.206f, 0.686f)
		//  Right : vec3(-0.701f, 0.000f, -0.713f)
		//  Up : vec3(-0.147f, 0.978f, 0.145f)
		//  Yaw : 135.465f
		//  Pitch : -11.914f
		//  Zoom : -2.911f
		//  Height : 10.000f
		//  -------------------------------- -
		//  ---- - camera--------------------
		//  Position : vec3(32.899f, 18.428f, 21.133f)
		//  Front : vec3(-0.822f, -0.211f, -0.528f)
		//  Right : vec3(0.540f, 0.000f, -0.841f)
		//  Up : vec3(-0.177f, 0.978f, -0.114f)
		//  Yaw : 212.715f
		//  Pitch : -12.164f
		//  Zoom : -2.911f
		//  Height : 10.000f
		//  -------------------------------- -

		SceneSlope->Camera->Position += 0.00051f * (vec3(32.899f, 18.428f, 21.133f) - vec3(27.899f, 18.257f, -27.450f));
		SceneSlope->Camera->Front += 0.00051f * (vec3(-0.822f, -0.211f, -0.528f) - vec3(-0.697f, -0.206f, 0.686f));
		SceneSlope->Camera->Yaw += 0.00051f * (212.715f - 135.465f);
		SceneSlope->Camera->Pitch += 0.00051f * (-12.164f - -11.914f);
		UpdateCameraVectors(SceneSlope->Camera);

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
			if (t > n)
			{
				if (i > notesCount-1)
				{
					t = 0;
					n = 150;
					state = 2;
				}
				else
				{
					AddMarble(world, &marbles[i]);
					marbles[i].power = 0.03f;
					world.Marbles[world.Marbles.size() - 1]->Active = true;

					t = 0;
					i++;
					n = notesTimes[i];
				}
			}
			t++;

			UpdateWorld(world, 0.000002f * delta);
		}

		for (int k = 0; k < world.Marbles.size(); k++)
		{
			if (world.Marbles[k]->Position[0] > 30.0f)
			{
				world.Marbles.erase(world.Marbles.begin() + k);
			}
		}

		if (state == 2)
		{
			UpdateWorld(world, 0.000002f * delta);

			t++;
			if (t > n) state++;

		}

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

		projMatrix = vmath::perspective(45.0f + SceneSlope->Camera->Zoom, (float)width / (float)height, 0.1f, 100.0f);
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
		world.ground = 0.8f;

		const vec3 colors[] = {
			vec3(100.0f, 1.0f, 1.0f),
			vec3(100.0f, 60.0f, 1.0f),
			vec3(100.0f, 100.0f, 1.0f),
			vec3(1.0f, 100.0f, 1.0f),
			vec3(1.0f, 90.0f, 90.0f),
			vec3(1.0f, 1.0f, 100.0f),
			vec3(50.0f, 1.0f, 100.0f),
		};
		const int notes[] = { 0,1,2,3,4,5,6, /* static emitters */
			0,1,2,3,4,5,6,  6,5,4,3,2,1,0,

			0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6,
			6,6, 5,5, 4,4, 3,3, 2,2, 1,1, 0,0,

			0,1,2, 1,2,3, 2,3,4, 3,4,5, 4,5,6,
			6,5,4, 5,4,3, 4,3,2, 3,2,1, 2,1,0,

			/* Turkish March (Mozart) */
			2,1,0,1,3,
			3,2,1,2,4,
			5,4,3,4,
			6,5,4,5,6,5,4,5,6,
			4,6,5,4,3,4,5,4,3,4,5,4,3,2,0,

			2,1,0,1,3,
			3,2,1,2,4,
			5,4,3,4,
			6,5,4,5,6,5,4,5,6,
			4,6,5,4,3,4,5,4,3,4,5,4,3,2,0,

			/* closing */
			6,5,4,3,2,1,0,
			6,5,4,3,2,1,0,
			6,5,4,3,2,1,0,
			
		};

		static int _notesTimes[] = { 10, 10, 10, 10, 10, 10, 10, /* static emitters */
			10, 7, 7, 7, 7, 7, 7,   25, 7, 7, 7, 7, 7, 7,

			60, 10, 20, 10, 20, 10, 20, 10, 20, 10, 20, 10, 20, 10,
			30, 10, 20, 10, 20, 10, 20, 10, 20, 10, 20, 10, 20, 10,

			60, 7, 7,  15, 7, 7,  15, 7, 7,  15, 7, 7,  15, 7, 7,
			20, 7, 7,  15, 7, 7,  15, 7, 7,  15, 7, 7,  15, 7, 7,

			/* Turkish March (Mozart) */
			80, 8, 8, 8, 8,
			35, 8, 8, 8, 8,
			35, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8,
			30, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

			30, 8, 8, 8, 8,
			35, 8, 8, 8, 8,
			35, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8,
			30, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

			/* closing */
			80,3,3,3,3,3,3,
			20,6,6,6,6,6,6,
			20,12,12,12,12,12,12,

			150,
		};

		notesTimes = _notesTimes;
		notesCount = _ARRAYSIZE(notes);

		for (int i = 0; i < _ARRAYSIZE(notes); i++)
		{
            int idx = notes[i % _ARRAYSIZE(notes)];
			marbles[i].Radius = 1.0f;
			marbles[i].Mass = 10000.0f;
			marbles[i].mat = matMarble;
			marbles[i].Audio = audio[idx];
			marbles[i].Angle = 0.0f;
			marbles[i].Axis = vec3();
			marbles[i].rotate = mat4::identity();
			marbles[i].xAngle = genRand(-3.14f, 3.14f);
			marbles[i].yAngle = genRand(-3.14f, 3.14f);
			marbles[i].zAngle = genRand(-3.14f, 3.14f);
			marbles[i].power = 0.001f;
			marbles[i].Active = false;

			marbles[i].Color = 0.2f * colors[idx];

			float rnd = genRand(0.0f, 1.0f);
			if (rnd > 0.66f)
				marbles[i].mLetter = n1;
			else if (rnd > 0.33f)
				marbles[i].mLetter = n2;
			else
				marbles[i].mLetter = n3;

			if (idx == 0) marbles[i].Position = vec3(-9.0f, 18.0f, -9.0f);
			else if (idx == 1) marbles[i].Position = vec3(-9.0f, 18.0f, -6.0f);
			else if (idx == 2) marbles[i].Position = vec3(-9.0f, 18.0f, -3.0f);
			else if (idx == 3) marbles[i].Position = vec3(-9.0f, 18.0f, 0.0f);
			else if (idx == 4) marbles[i].Position = vec3(-9.0f, 18.0f, 3.0f);
			else if (idx == 5) marbles[i].Position = vec3(-9.0f, 18.0f, 6.0f);
			else if (idx == 6) marbles[i].Position = vec3(-9.0f, 18.0f, 9.0f);

			marbles[i].Velocity = genVec3(0.0030f, 0.0035f, -0.065f, -0.060f, 0.0010f, 0.0015f);
			marbles[i].Active = false;
		}

		marbles[0].mLetter = NULL;
		AddMarble(world, &marbles[0]);

		marbles[1].mLetter = NULL;
		AddMarble(world, &marbles[1]);

		marbles[2].mLetter = NULL;
		AddMarble(world, &marbles[2]);

		marbles[3].mLetter = NULL;
		AddMarble(world, &marbles[3]);

		marbles[4].mLetter = NULL;
		AddMarble(world, &marbles[4]);

		marbles[5].mLetter = NULL;
		AddMarble(world, &marbles[5]);

		marbles[6].mLetter = NULL;
		AddMarble(world, &marbles[6]);
		
		walls[0].Normal = normalize(vec3(0.5f, 1.0f, 0.0f));
		walls[0].D = -0.5f;
		walls[0].force = vec3(100.0f, 0.0f, 0.0f);
		walls[0].force = vec3(0.0f);

		AddWall(world, &walls[0]);

	}
}

Scene *GetSlopeScene()
{
	if (!SceneSlope)
	{
		SceneSlope = (Scene*)malloc(sizeof(Scene));

		strcpy_s(SceneSlope->Name, "SlopeScene");

		SceneSlope->InitFunc   = slope::Init;
		SceneSlope->UninitFunc = slope::Uninit;
		SceneSlope->ResetFunc  = slope::Reset;

		SceneSlope->DisplayFunc = slope::Display;
		SceneSlope->UpdateFunc  = slope::Update;
		SceneSlope->ResizeFunc  = slope::Resize;

		//  ---- - camera--------------------
		//  Position: vec3(27.899f, 18.257f, -27.450f)
		//  Front : vec3(-0.697f, -0.206f, 0.686f)
		//  Right : vec3(-0.701f, 0.000f, -0.713f)
		//  Up : vec3(-0.147f, 0.978f, 0.145f)
		//  Yaw : 135.465f
		//  Pitch : -11.914f
		//  Zoom : -2.911f
		//  Height : 10.000f
		//  -------------------------------- -
		//  ---- - camera--------------------
		//  Position : vec3(32.899f, 18.428f, 21.133f)
		//  Front : vec3(-0.822f, -0.211f, -0.528f)
		//  Right : vec3(0.540f, 0.000f, -0.841f)
		//  Up : vec3(-0.177f, 0.978f, -0.114f)
		//  Yaw : 212.715f
		//  Pitch : -12.164f
		//  Zoom : -2.911f
		//  Height : 10.000f
		//  -------------------------------- -

		SceneSlope->Camera = AddNewCamera(
			vec3(27.899f, 18.257f, -27.450f),
			vec3(-0.697f, -0.206f, 0.686f),
			vec3(0.0f, 1.0f, 0.0f),
			135.0f, -11.50f,
			-2.9f, 10.0f);
	}

	return SceneSlope;
}
