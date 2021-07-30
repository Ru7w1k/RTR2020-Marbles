#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "rigidBody.h"

#include "primitives.h"
#include "PBRShader.h"

#include <set>

// helper functions
float distance(vec3& point, Wall* wall)
{
	return fabsf((wall->Normal[0] * point[0])
		+ (wall->Normal[1] * point[1])
		+ (wall->Normal[2] * point[2])
		+ wall->D);
}


// constructor functions
Marble* NewMarble()
{
	return NULL;
}

Wall* NewWall(vec3 minPoint, vec3 maxPoint)
{
	return NULL;
}



// rigid body functions
void AddMarble(World& world, Marble* marble)
{
	world.Marbles.push_back(marble);
}

void AddWall(World& world, Wall* wall)
{
	world.Walls.push_back(wall);
}

void AddBox(World& world, Box* box)
{
	world.Boxes.push_back(box);
}

void DrawWorld(World& world)
{
	static float t = 0.0f;
	t += 1.0f;

	vector<vec3> lightPos;
	vector<vec3> lightCol;

	PBRShaderUniforms* u = UsePBRShader();
	for (int i = 0; i < world.Marbles.size(); i++)
	{
		lightPos.push_back(world.Marbles[i]->Position);
		lightCol.push_back(world.Marbles[i]->Color);
	}


	glUniform3fv(u->lightPosUniform, (GLsizei)world.Marbles.size(), (GLfloat *)lightPos.data());
	glUniform3fv(u->lightColUniform, (GLsizei)world.Marbles.size(), (GLfloat *)lightCol.data());


	for (int i = 0; i < world.Marbles.size(); i++)
	{
		mat4 modelMat = translate(world.Marbles[i]->Position);
		modelMat *= rotate(t, 1.0f, 0.0f, 0.0f);
		modelMat *= rotate(t+2.0f, 0.0f, 1.0f, 0.0f);
		modelMat *= rotate(t-1.5f, 0.0f, 0.0f, 1.0f);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMat);
		useMaterial(world.Marbles[i]->mat);
		DrawSphere();
	}

	/*for (int i = 0; i < world.Walls.size(); i++)
	{
		mat4 modelMat = translate(world.Walls[i].);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMat);
		DrawPlane();
	}*/
}

void UpdateWorld(World& world, float time)
{
	set<ALuint>collided;

	// delta steps per frame
	for (int t = 0; t < 20; t++)
	{
		// for each marble, move to next position
		for (int i = 0; i < world.Marbles.size(); i++)
		{
			// calculate the next position of marble
			vec3 F = vec3(0.0f, -0.001f, 0.0f);
			vec3 a = F / world.Marbles[i]->Mass;
			vec3 s = world.Marbles[i]->Velocity * time + 0.5f * a * time * time;
			vec3 v = world.Marbles[i]->Velocity + a * time;

			world.Marbles[i]->Velocity = v;
			world.Marbles[i]->Position = world.Marbles[i]->Position + s;
		}

		// for each marble resolve collisions
		for (int i = 0; i < world.Marbles.size(); i++)
		{
			// collision check with all walls
			for (int j = 0; j < world.Walls.size(); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Walls[j]);
				if (d < world.Marbles[i]->Radius)
				{
					world.Marbles[i]->Position -= (world.Marbles[i]->Radius - d) * normalize(world.Marbles[i]->Velocity);
					world.Marbles[i]->Velocity = 0.8f * reflect(world.Marbles[i]->Velocity, world.Walls[j]->Normal);
					
					if ((world.Marbles[i]->Radius - d) >= 0.01f)
					{
						collided.insert(world.Marbles[i]->Audio);
					}
				}
			}

			// collision with other marbles
			for (int j = i+1; j < world.Marbles.size(); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Marbles[j]->Position);
				vec3 N = normalize(world.Marbles[j]->Position - world.Marbles[i]->Position);

				if (d < world.Marbles[i]->Radius + world.Marbles[j]->Radius)
				{
					vec3 vA = 0.8f * reflect(world.Marbles[i]->Velocity,  N) + 0.1f * world.Marbles[j]->Velocity ;
					vec3 vB = 0.8f * reflect(world.Marbles[j]->Velocity, -N) + 0.1f * world.Marbles[i]->Velocity ;

					world.Marbles[i]->Velocity = vA;
					world.Marbles[j]->Velocity = vB;

					world.Marbles[i]->Position += 0.5f * (world.Marbles[i]->Radius + world.Marbles[j]->Radius - d) * -N;
					world.Marbles[j]->Position += 0.5f * (world.Marbles[i]->Radius + world.Marbles[j]->Radius - d) * N;

					if (d >= 0.01f)
					{
						collided.insert(world.Marbles[i]->Audio);
						collided.insert(world.Marbles[j]->Audio);
					}
				}
			}
		}
	}

	for (ALuint id : collided)
	{
		PlayAudio(id);
	}

}

void DeleteWorld(World& world)
{

}

void ResetWorld(World& world)
{
	world.Marbles.clear();
	world.Walls.clear();
	world.Boxes.clear();
}

