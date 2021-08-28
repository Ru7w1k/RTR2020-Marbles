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
Scene *SceneMarbles = NULL;

namespace marbles
{
	// scene state
	int gWidth, gHeight;
	mat4 projMatrix;

	Material *matPlastic = NULL;
	Material *matMarble = NULL;
	Material *matGround = NULL;

	World world;
	Marble marbles[7];
	Wall walls[10];
	ALuint audio[8];

	Framebuffer *fboMain = NULL;
	Framebuffer *fboPingpong[2] = {NULL, NULL};
	
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

		matPlastic = loadMaterial("res\\materials\\plastic");
		matMarble = loadMaterial("res\\materials\\marble");
		matGround= loadMaterial("res\\materials\\wood");

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

		world.cam = SceneMarbles->Camera;
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

		alListenerfv(AL_POSITION, SceneMarbles->Camera->Position);
		alListenerfv(AL_ORIENTATION, new ALfloat[]{ SceneMarbles->Camera->Position[0], SceneMarbles->Camera->Position[1], SceneMarbles->Camera->Position[2], 0.0f, 1.0f, 0.0f });

		projMatrix = vmath::perspective(45.0f + SceneMarbles->Camera->Zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

		//TextureShaderUniforms *ut = UseTextureShader();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, skyTex);
		//glUniform1i(ut->samplerUniform, 0);
		//glUniform1f(ut->scaleUniform, 5.0f);

		//glCullFace(GL_FRONT);
		//glUniformMatrix4fv(ut->mvpMatrixUniform, 1, GL_FALSE, projMatrix * GetViewMatrixNoTranslate(SceneMarbles->Camera) * scale(80.0f, 80.0f, 80.0f));
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
		glUniformMatrix4fv(u->vMatrixUniform, 1, GL_FALSE, GetViewMatrix(SceneMarbles->Camera));
		glUniformMatrix4fv(u->pMatrixUniform, 1, GL_FALSE, projMatrix);

		glUniform3fv(u->cameraPosUniform, 1, SceneMarbles->Camera->Position);
		
		glUniform1f(u->alpha, 1.0f);

		//modelMatrix = scale(100.0f, 0.5f, 100.0f);
		//glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMatrix);
		//useMaterial(matGround);
		//DrawCube();

		modelMatrix = scale(20.0f, 0.5f, 20.0f);
		useMaterial(matPlastic);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, rotate(45.0f, 1.0f, 0.0f, 0.0f) * modelMatrix);
		DrawCube();

		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, rotate(-45.0f, 1.0f, 0.0f, 0.0f) * modelMatrix);
		DrawCube();

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
		static int i = 0;
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
			if (t > 4)
			{
				if (i > 6)
				{
					state = 2;
				}
				else
				{
					world.Marbles[i]->Active = true;
					t = 0;
					i++;
				}
			}
			t++;

			SceneMarbles->Camera->Position += -0.0012f * (vec3(37.515f, 24.477f, 0.115f) - vec3(0.063611f, 42.330196f, -7.289144f));
			SceneMarbles->Camera->Front += -0.0012f * (vec3(-0.938f, -0.347f, -0.003f) - vec3(-0.001590f, -0.983255f, 0.182229f));
			SceneMarbles->Camera->Up += -0.0012f * (vec3(-0.347f, 0.938f, -0.001f) - vec3(-0.008580f, 0.182236f, 0.983218f));
			SceneMarbles->Camera->Yaw += -0.0012f * (180.175f - 90.50f);
			SceneMarbles->Camera->Pitch += -0.0012f * (-20.0f - -79.50f);
			SceneMarbles->Camera->Zoom += -0.0012f * (-5.6f - -20.0f);
			SceneMarbles->Camera->Height += -0.0012f * (10.6f - 3.0f);

		}



		if (state == 2)
		{
			/*SceneMarbles->Camera = AddNewCamera(
				vec3(37.515f, 24.477f, 0.115f),
				vec3(-0.938f, -0.347f, -0.003f),
				vec3(-0.347f, 0.938f, -0.001f),
				180.175f, -20.0f,
				-5.6f, 10.6f);

			SceneMarbles->Camera = AddNewCamera(
				vec3(0.063611f, 42.330196f, -7.289144f),
				vec3(-0.001590f, -0.983255f, 0.182229f),
				vec3(-0.008580f, 0.182236f, 0.983218f),
				90.50f, -79.50f,
				-20.0f, 3.0f);*/

			SceneMarbles->Camera->Position += -0.0012f * (vec3(37.515f, 24.477f, 0.115f) - vec3(0.063611f, 42.330196f, -7.289144f));
			SceneMarbles->Camera->Front += -0.0012f * (vec3(-0.938f, -0.347f, -0.003f) - vec3(-0.001590f, -0.983255f, 0.182229f));
			SceneMarbles->Camera->Up += -0.0012f * (vec3(-0.347f, 0.938f, -0.001f) - vec3(-0.008580f, 0.182236f, 0.983218f));
			SceneMarbles->Camera->Yaw += -0.0012f * (180.175f - 90.50f);
			SceneMarbles->Camera->Pitch += -0.0012f * (-20.0f - -79.50f);
			SceneMarbles->Camera->Zoom += -0.0012f * (-5.6f - -20.0f);
			SceneMarbles->Camera->Height += -0.0012f * (10.6f - 3.0f);

			if (SceneMarbles->Camera->Zoom <= -20.5f) state++;
		}

		if (state == 3)
		{
			fadeV += 0.01f;
			if (fadeV >= 1.0f)
			{
				fadeV = 1.0f;
				return true;
			}
		}

		UpdateWorld(world, 0.000002f * delta);
		return false;
	}

	void Resize(int width, int height)
	{
		const float dim = 50.0f;
		
		gWidth = width;
		gHeight = height;

		projMatrix = vmath::perspective(45.0f + SceneMarbles->Camera->Zoom, (float)width / (float)height, 0.1f, 100.0f);
		ResizeFramebuffer(fboMain, width, height);
		ResizeFramebuffer(fboPingpong[0], width, height);
		ResizeFramebuffer(fboPingpong[1], width, height);
	}

	void Reset(void)
	{
		ResetWorld(world);

		fadeV = 1.0f;
		state = 0;
		world.ground = 0.8f;

		for (int i = 0; i < 7; i++)
		{
			marbles[i].Position = vec3(12.0f - (i * 3.5f), 17.50f, 0.0f);
			marbles[i].Color = vec3(100.0f, 100.0f, 1.0f);
			marbles[i].Radius = 1.0f;
			marbles[i].Velocity = vec3(0.0f, 0.001f, 0.05f);
			marbles[i].Mass = 10000.0f;
			marbles[i].mat = matMarble;
			marbles[i].Audio = audio[i % 7];
			marbles[i].Angle = 0.0f;
			marbles[i].Axis = vec3();
			marbles[i].rotate = mat4::identity();
			marbles[i].xAngle = radians(-90.0f);
			marbles[i].yAngle = 0.0f;
			marbles[i].zAngle = 0.0f;
			marbles[i].power = 0.01f;
			marbles[i].Active = false;
		}

		marbles[0].mLetter = GetModel('M');
		marbles[1].mLetter = GetModel('A');
		marbles[2].mLetter = GetModel('R');
		marbles[3].mLetter = GetModel('B');
		marbles[4].mLetter = GetModel('L');
		marbles[5].mLetter = GetModel('E');
		marbles[6].mLetter = GetModel('S');

		AddMarble(world, &marbles[0]);
		AddMarble(world, &marbles[1]);
		AddMarble(world, &marbles[2]);
		AddMarble(world, &marbles[3]);
		AddMarble(world, &marbles[4]);
		AddMarble(world, &marbles[5]);
		AddMarble(world, &marbles[6]);
		
		walls[0].Normal = normalize(vec3(0.0f, 1.0f, 0.0f));
		walls[0].D = -0.5f;

		walls[1].Normal = normalize(vec3(0.0f, 1.0f, 1.0f));
		walls[1].D = -0.5f;

		walls[2].Normal = normalize(vec3(0.0f, 1.0f, -1.0f));
		walls[2].D = -0.5f;

		AddWall(world, &walls[0]);
		AddWall(world, &walls[1]);
		AddWall(world, &walls[2]);
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

		// [2963365429248.000000] ---- - camera--------------------
		// [2963365429248.000000] Position: 0.063611 42.330196 - 7.289144
		// [2963365429248.000000] Front : -0.001590 - 0.983255 0.182229
		// [2963365429248.000000] Right : -0.999962 0.000000 - 0.008727
		// [2963365429248.000000] Up : -0.008580 0.182236 0.983218
		// [2963365429248.000000] Yaw : 90.500000
		// [2963365429248.000000] Pitch : -79.500000
		// [2963365429248.000000] Zoom : -20.000000
		// [2963365429248.000000] Height : 3.000000
		// [2963365429248.000000] -------------------------------- -
		// [2965107376128.000000] ---- - camera--------------------
		// [2965107376128.000000] Position : 28.342886 32.172813 - 12.767541
		// [2965107376128.000000] Front : -0.708572 - 0.629320 0.319189
		// [2965107376128.000000] Right : -0.410719 0.000000 - 0.911762
		// [2965107376128.000000] Up : -0.573790 0.777146 0.258474
		// [2965107376128.000000] Yaw : -204.250000
		// [2965107376128.000000] Pitch : -39.000000
		// [2965107376128.000000] Zoom : -4.000000
		// [2965107376128.000000] Height : 7.000000
		// [2965107376128.000000] -------------------------------- -
		// [3010305458176.000000] ---- - camera--------------------
		// [3010305458176.000000] Position: vec3(37.515f, 24.477f, 0.115f)
		// [3010305458176.000000] Front : vec3(-0.938f, -0.347f, -0.003f)
		// [3010305458176.000000] Right : vec3(0.003f, 0.000f, -1.000f)
		// [3010305458176.000000] Up : vec3(-0.347f, 0.938f, -0.001f)
		// [3010305458176.000000] Yaw : 180.175f
		// [3010305458176.000000] Pitch : -20.300f
		// [3010305458176.000000] Zoom : -5.600f
		// [3010305458176.000000] Height : 10.600f
		// [3010305458176.000000] -------------------------------- -



		/*SceneMarbles->Camera = AddNewCamera(
			vec3(28.342886f, 32.172813f, -12.767541f),
			vec3(-0.708572f, -0.629320f, 0.319189f),
			vec3(-0.573790f, 0.777146f, 0.258474f),
			156.25f, -39.0f,
			-4.0f, 7.0f);*/

		SceneMarbles->Camera = AddNewCamera(
			vec3(37.515f, 24.477f, 0.115f),
			vec3(-0.938f, -0.347f, -0.003f),
			vec3(-0.347f, 0.938f, -0.001f),
			180.175f, -20.0f,
			-5.6f, 10.6f);

	}

	return SceneMarbles;
}
