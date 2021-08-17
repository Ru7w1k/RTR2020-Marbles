#pragma once

#include "main.h"
#include "material.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace std;

typedef unsigned int  uint;

// types for parsing model
// vertex data of animated model
struct Vertex
{
	vec3 position;
	vec3 normal;
	vec2 uv;
	ivec4 boneIDs = { 0, 0, 0, 0 };
	vec4 boneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
};

// struct to hold bone tree i.e. skeleton
struct Bone
{
	int id = 0;
	string name = "";
	mat4 offset = mat4::identity();
	vector<Bone> child = {};
};

// struct to represent an animation track
struct BoneTransformTrack
{
	vector<float> positionTimestamps = {};
	vector<float> rotationTimestamps = {};
	vector<float> scaleTimestamps = {};

	vector<vec3> positions = {};
	vector<vec4> rotations = {};
	vector<vec3> scales = {};
};

// struct to contain the animation information
struct Animation
{
	float duration = 0.0f;
	float ticksPerSecond = 1.0f;
	unordered_map<string, BoneTransformTrack> *boneTransform;
};


typedef struct _Model 
{
	// mesh
	GLuint vao;
	GLuint vbo;
	GLuint vboIdx;

	GLsizei idxCount;

	// textures
	bool     useDiffMap;
	GLuint   diffMap;
	Material pbrMaps;

	// animation data
	bool      isAnimated;
	Bone      skeleton;
	uint      boneCount;
	Animation *anim;
	mat4      globalInvTransform;

} Model;

Model* LoadModel(const char* path, bool animate = false);
void GetPose(Model* pModel, float dt, vector<mat4>& outputPos);
void DrawModel(Model* pModel);
void FreeModel(Model* pModel);
