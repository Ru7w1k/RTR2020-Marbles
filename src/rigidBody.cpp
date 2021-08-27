#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

#include "rigidBody.h"

#include "primitives.h"
#include "PBRShader.h"

#include <set>
#include <map>

// helper functions
float distance(vec3& point, Wall* wall)
{
	return fabsf((wall->Normal[0] * point[0])
		+ (wall->Normal[1] * point[1])
		+ (wall->Normal[2] * point[2])
		+ wall->D);
}

bool withinRange(Marble* marble, Wall* wall)
{
	return marble->Position[0] >= wall->MinPoint[0] + marble->Radius
		//&& marble->Position[1] >= wall->MinPoint[1] + marble->Radius
		&& marble->Position[2] >= wall->MinPoint[2] + marble->Radius
		&& marble->Position[0] <= wall->MaxPoint[0] + marble->Radius
		//&& marble->Position[1] <= wall->MaxPoint[1] + marble->Radius
		&& marble->Position[2] <= wall->MaxPoint[2] + marble->Radius;
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
	map<float, int> sortedMarbles;

	vector<vec3> lightPos;
	vector<vec3> lightCol;

	PBRShaderUniforms* u = UsePBRShader();
	for (int i = 0; i < world.Marbles.size(); i++)
	{
		lightPos.push_back(world.Marbles[i]->Position);
		lightCol.push_back(70.0f * world.Marbles[i]->power * world.Marbles[i]->Color);
	}

	glUniform1i(u->lightCountUniform, (GLint)world.Marbles.size());
	glUniform3fv(u->lightPosUniform, (GLsizei)world.Marbles.size(), (GLfloat *)lightPos.data());
	glUniform3fv(u->lightColUniform, (GLsizei)world.Marbles.size(), (GLfloat *)lightCol.data());

	// sort marbles wrt depth for bleding
	for (int i = 0; i < world.Marbles.size(); i++)
	{
		float d = distance(world.cam->Position, world.Marbles[i]->Position);
		sortedMarbles[d] = i;
	}

	// draw
	for (map<float, int>::reverse_iterator it = sortedMarbles.rbegin(); it != sortedMarbles.rend(); it++)
	{
		mat4 modelMat = translate(world.Marbles[it->second]->Position);
		modelMat *= vmath::rotateR(world.Marbles[it->second]->xAngle, world.Marbles[it->second]->yAngle, world.Marbles[it->second]->zAngle);
		glUniformMatrix4fv(u->mMatrixUniform, 1, GL_FALSE, modelMat);
		useMaterial(world.Marbles[it->second]->mat);
		
		glUniform1f(u->alpha, 1.0f);
		glUniform1i(u->bright, 1);
		glUniform4fv(u->brightColor, 1, vec4(world.Marbles[it->second]->power * world.Marbles[it->second]->Color, 1.0f));
		DrawModel(world.Marbles[it->second]->mLetter);

		glUniform1i(u->bright, 0);
		glUniform1f(u->alpha, 0.5f);
		DrawSphere();

		if (world.Marbles[it->second]->power > 0.01f)
		{
			world.Marbles[it->second]->power -= 0.0025f;
		}
	}

}

void UpdateWorld(World& world, float time)
{
	set<int>collided;

	// delta steps per frame
	for (int t = 0; t < 20; t++)
	{
		// for each marble, move to next position
		for (int i = 0; i < world.Marbles.size(); i++)
		{
			if (!world.Marbles[i]->Active) continue;

			// calculate the next position of marble
			vec3 F = vec3(0.0f, -1.0f, 0.0f);
			vec3 a = F / world.Marbles[i]->Mass;
			vec3 s = world.Marbles[i]->Velocity * time + 0.5f * a * time * time;
			vec3 v = world.Marbles[i]->Velocity + a * time;

			world.Marbles[i]->Velocity = v;
			world.Marbles[i]->Position = world.Marbles[i]->Position + s;

			if (world.Marbles[i]->Roll)
			{
				world.Marbles[i]->Angle = length(s);

				mat4 r = rotateR(world.Marbles[i]->Angle, world.Marbles[i]->Axis);
				world.Marbles[i]->Angle = 0.0f;

				float sy = sqrtf(r[1][2] * r[1][2] + r[2][2] * r[2][2]);
				// not singular
				if (sy >= 1e-6)
				{
					world.Marbles[i]->xAngle += atan2f(r[1][2], r[2][2]);
					world.Marbles[i]->yAngle += atan2f(-r[0][2], sy);
					world.Marbles[i]->zAngle += atan2f(r[0][1], r[0][0]);
				}
				// singular
				else
				{
					world.Marbles[i]->xAngle += atan2f(-r[2][1], r[1][1]);
					world.Marbles[i]->yAngle += atan2f(-r[0][2], sy);
					// world.Marbles[it->second]->zAngle += 0.0f;
				}
			}
		}

		// for each marble resolve collisions
		for (int i = 0; i < world.Marbles.size(); i++)
		{
			if (!world.Marbles[i]->Active) continue;

			// collision check with all infinite large walls
			for (int j = 0; j < vmath::min(5,(int)world.Walls.size()); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Walls[j]);
				if (d <= world.Marbles[i]->Radius)
				{
					world.Marbles[i]->Position -= (world.Marbles[i]->Radius - d) * normalize(world.Marbles[i]->Velocity);
					world.Marbles[i]->Velocity = 0.8f * reflect(world.Marbles[i]->Velocity, world.Walls[j]->Normal);

					if (length(world.Marbles[i]->Velocity) > 0.001f)
					{
						//world.Marbles[i]->rotate = rotate(world.Marbles[i]->Angle * 57.2957f, world.Marbles[i]->Axis);
						vec3 c = cross(world.Walls[j]->Normal, normalize(world.Marbles[i]->Velocity));
						if (length2(c) == 0.0f)
						{
							world.Marbles[i]->Roll |= false;
						}
						else
						{
							world.Marbles[i]->Roll = true;
							world.Marbles[i]->Axis = normalize(c);
						}
						
						collided.insert(i);
					}
					else
					{
						world.Marbles[i]->Velocity = vec3(0.0f);
					}
				}
			}

			// collision check with all finite walls
			for (int j = 5; j < world.Walls.size(); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Walls[j]);
				if (d <= world.Marbles[i]->Radius && withinRange(world.Marbles[i], world.Walls[j]))
				{
					world.Marbles[i]->Position -= (world.Marbles[i]->Radius - d) * normalize(world.Marbles[i]->Velocity);
					world.Marbles[i]->Velocity = 0.8f * reflect(world.Marbles[i]->Velocity, world.Walls[j]->Normal);

					if (length(world.Marbles[i]->Velocity) > 0.001f)
					{
						collided.insert(i);
					}
					else
					{
						world.Marbles[i]->Velocity = vec3(0.0f);
					}
				}
			}

			// collision check with all boxes
			for (int j = 5; j < world.Boxes.size(); j++)
			{

			}

			// collision with other marbles
			for (int j = i+1; j < world.Marbles.size(); j++)
			{
				float d = distance(world.Marbles[i]->Position, world.Marbles[j]->Position);
				vec3 N = normalize(world.Marbles[j]->Position - world.Marbles[i]->Position);

				if (d <= world.Marbles[i]->Radius + world.Marbles[j]->Radius)
				{
					if (length(world.Marbles[i]->Velocity) > 0.001f)
					{
						collided.insert(i);
					}
					else
					{
						world.Marbles[i]->Velocity = vec3(0.0f);
					}

					if (length(world.Marbles[j]->Velocity) > 0.001f)
					{
						collided.insert(j);
					}
					else
					{
						world.Marbles[j]->Velocity = vec3(0.0f);
					}


					vec3 vA = 0.7f * reflect(world.Marbles[i]->Velocity,  N) + 0.3f * world.Marbles[j]->Velocity ;
					vec3 vB = 0.7f * reflect(world.Marbles[j]->Velocity, -N) + 0.3f * world.Marbles[i]->Velocity ;

					world.Marbles[i]->Velocity = vA;
					world.Marbles[j]->Velocity = vB;

					world.Marbles[i]->Position += 0.5f * (world.Marbles[i]->Radius + world.Marbles[j]->Radius - d) * -N;
					world.Marbles[j]->Position += 0.5f * (world.Marbles[i]->Radius + world.Marbles[j]->Radius - d) * N;

					vec3 c = cross(N, normalize(world.Marbles[i]->Velocity));
					if (length2(c) == 0.0f)
					{
						world.Marbles[i]->Roll |= false;
					}
					else
					{
						world.Marbles[i]->Roll = true;
						world.Marbles[i]->Axis = normalize(c);
					}

					c = cross(-N, normalize(world.Marbles[j]->Velocity));
					if (length2(c) == 0.0f)
					{
						world.Marbles[j]->Roll |= false;
					}
					else
					{
						world.Marbles[j]->Roll = true;
						world.Marbles[j]->Axis = normalize(c);
					}
					
				}
			}
		}
	}

	for (int id : collided)
	{
		alSourcefv(world.Marbles[id]->Audio, AL_POSITION, -world.Marbles[id]->Position);
		PlayAudio(world.Marbles[id]->Audio);
		world.Marbles[id]->power = 0.08f;
	}
}

void DeleteWorld(World& world)
{
	world.Marbles.clear();
	world.Walls.clear();
	world.Boxes.clear();
	world.cam = NULL;
}

void ResetWorld(World& world)
{
	world.Marbles.clear();
	world.Walls.clear();
	world.Boxes.clear();
}

