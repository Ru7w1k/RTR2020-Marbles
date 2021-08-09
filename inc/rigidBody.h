#pragma once
#include "main.h"
#include "Camera.h"

#include <vector>
using namespace std;

#include "material.h"

typedef struct _Sphere
{
	vec3 Position;
	float Radius;

	float Mass;
	vec3 Velocity;

	bool Roll;
	float Angle;
	vec3 Axis;

	vec3 Color;
	Material* mat;

	ALuint Audio;

} Marble;

typedef struct _Plane
{
	vec3 Normal;
	float D;

	vec3 MinPoint;
	vec3 MaxPoint;

} Wall;

typedef struct _Cube
{

} Box;


typedef struct _World
{
	vector<Marble*> Marbles;
	vector<Wall*> Walls;
	vector<Box*> Boxes;

	Camera* cam;

} World;

void AddMarble(World& world, Marble* marble);
void AddWall(World& world, Wall* wall);
void AddBox(World& world, Box* box);

void DrawWorld(World& world);
void UpdateWorld(World& world, float time);

void DeleteWorld(World& world);
void ResetWorld(World& world);

// helper functions
float distance(vec3& point, Wall* wall);
