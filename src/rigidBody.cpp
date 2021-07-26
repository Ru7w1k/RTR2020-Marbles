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
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMat);
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
				if (d <= world.Marbles[i]->Radius)
				{
					world.Marbles[i]->Velocity = 0.8f * reflect(world.Marbles[i]->Velocity, world.Walls[j]->Normal);
					world.Marbles[i]->Position += (world.Marbles[i]->Radius - d) * world.Walls[j]->Normal;
				}
			}

			// collision with other marbles
			for (int j = i+1; j < world.Marbles.size(); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Marbles[j]->Position);
				vec3 N = normalize(world.Marbles[j]->Position - world.Marbles[i]->Position);

				/*vec3 k = normalize(world.Marbles[i]->Position - world.Marbles[j]->Position);
				float a =  dot(2.0f * k, (world.Marbles[i]->Velocity - world.Marbles[j]->Velocity)) /
					((1.0f / world.Marbles[i]->Mass) + (1.0f / world.Marbles[j]->Mass));*/

				if (d <= world.Marbles[i]->Radius + world.Marbles[j]->Radius)
				{
					//world.Marbles[i]->Velocity = world.Marbles[i]->Velocity - ((a / world.Marbles[i]->Mass) * k);
					//world.Marbles[j]->Velocity = world.Marbles[j]->Velocity - ((a / world.Marbles[j]->Mass) * k);

					world.Marbles[i]->Velocity = 0.8f * reflect(world.Marbles[i]->Velocity, N);
					world.Marbles[j]->Velocity = 0.8f * reflect(world.Marbles[j]->Velocity, N);

					world.Marbles[i]->Position += (world.Marbles[i]->Radius - (0.5f * d)) * -N;
					world.Marbles[j]->Position += (world.Marbles[j]->Radius - (0.5f * d)) * N;
				}
			}
		}
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

