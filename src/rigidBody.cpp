#include "main.h"
#include "helper.h"
#include "logger.h"

#include "rigidBody.h"

#include "primitives.h"
#include "PBRShader.h"


// helper functions
float distance(vec3& point, Wall* wall)
{
	return fabsf((wall->Normal[0] * point[0])
		+ (wall->Normal[1] * point[1])
		+ (wall->Normal[2] * point[2])
		+ wall->D);
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
	PBRShaderUniforms* u = UsePBRShader();
	for (int i = 0; i < world.Marbles.size(); i++)
	{
		mat4 modelMat = translate(world.Marbles[i]->Position);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMat);
		DrawSphere();
	}
}

void UpdateWorld(World& world, float time)
{
	// delta steps per frame
	for (int t = 0; t < 20; t++)
	{
		// for each marble
		for (int i = 0; i < world.Marbles.size(); i++)
		{
			// calculate the next position of marble
			vec3 F = vec3(0.0f, -0.001f, 0.0f);
			vec3 a = F / world.Marbles[i]->Mass;
			vec3 s = world.Marbles[i]->Velocity * time + 0.5f * a * time * time;
			vec3 v = world.Marbles[i]->Velocity + a * time;

			world.Marbles[i]->Velocity = v;
			world.Marbles[i]->Position = world.Marbles[i]->Position + s;

			// collision check with all walls
			for (int j = 0; j < world.Walls.size(); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Walls[j]);
				if (d <= world.Marbles[i]->Radius)
				{
					world.Marbles[i]->Velocity = 0.8f * reflect(world.Marbles[i]->Velocity, world.Walls[j]->Normal);
					world.Marbles[i]->Position += (world.Marbles[i]->Radius - d) * world.Walls[j]->Normal;
				}
			}
		}
	}
}

void DeleteWorld(World& world)
{

}

