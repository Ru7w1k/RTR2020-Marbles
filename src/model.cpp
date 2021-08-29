// headers
#include "main.h"
#include "model.h"

// public members to this object
static Assimp::Importer importer;

// helper functions
mat4 assimpTomat4(aiMatrix4x4 m)
{
	return mat4(
		vec4(m.a1, m.b1, m.c1, m.d1),
		vec4(m.a2, m.b2, m.c2, m.d2),
		vec4(m.a3, m.b3, m.c3, m.d3),
		vec4(m.a4, m.b4, m.c4, m.d4)
	);
}

vec3 assimpTovec3(aiVector3D v)
{
	return vec3(v.x, v.y, v.z);
}

vec4 assimpTovec4(aiQuaternion q)
{
	return vec4(q.x, q.y, q.z, q.w);
}

vec3 lerp(vec3 a, vec3 b, float t)
{
	return ((1.0f - t) * a) + (b * t);
}

vec4 quatSlerp(vec4 q1, vec4 q2, float t)
{
	float cosom = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];

	if (cosom < 0.0f)
	{
		cosom = -cosom;
		q2 *= -1.0f;
	}

	float sclq1, sclq2;
	if ((1.0f - cosom) > 0.0001f)
	{
		// standard case (slerp)
		float omega, sinom;
		omega = acosf(cosom);
		sinom = sinf(omega);
		sclq1 = sin((1.0f - t) * omega) / sinom;
		sclq2 = sin(t * omega) / sinom;
	}
	else
	{
		// very close, perform linear interpolation
		sclq1 = 1.0f - t;
		sclq2 = t;
	}

	return (sclq1 * q1) + (sclq2 * q2);
}

mat4 affineTransform(vec3 scale, quaternion rotate, vec3 translate)
{
	mat4 m = rotate.asMatrix();

	m[0][0] *= scale[0];
	m[1][0] *= scale[0];
	m[2][0] *= scale[0];
	m[3][0] = translate[0];

	m[0][1] *= scale[1];
	m[1][1] *= scale[1];
	m[2][1] *= scale[1];
	m[3][1] = translate[1];

	m[0][2] *= scale[2];
	m[1][2] *= scale[2];
	m[2][2] *= scale[2];
	m[3][2] = translate[2];

	return m;
}

float determinant(mat4 m)
{
	return m[0][1] * m[1][2] * m[2][3] * m[3][4] - m[0][1] * m[1][2] * m[2][4] * m[3][3] + m[0][1] * m[1][3] * m[2][4] * m[3][2] - m[0][1] * m[1][3] * m[2][2] * m[3][4]
		+ m[0][1] * m[1][4] * m[2][2] * m[3][3] - m[0][1] * m[1][4] * m[2][3] * m[3][2] - m[0][2] * m[1][3] * m[2][4] * m[3][1] + m[0][2] * m[1][3] * m[2][1] * m[3][4]
		- m[0][2] * m[1][4] * m[2][1] * m[3][3] + m[0][2] * m[1][4] * m[2][3] * m[3][1] - m[0][2] * m[1][1] * m[2][3] * m[3][4] + m[0][2] * m[1][1] * m[2][4] * m[3][3]
		+ m[0][3] * m[1][4] * m[2][1] * m[3][2] - m[0][3] * m[1][4] * m[2][2] * m[3][1] + m[0][3] * m[1][1] * m[2][2] * m[3][4] - m[0][3] * m[1][1] * m[2][4] * m[3][2]
		+ m[0][3] * m[1][2] * m[2][4] * m[3][1] - m[0][3] * m[1][2] * m[2][1] * m[3][4] - m[0][4] * m[1][1] * m[2][2] * m[3][3] + m[0][4] * m[1][1] * m[2][3] * m[3][2]
		- m[0][4] * m[1][2] * m[2][3] * m[3][1] + m[0][4] * m[1][2] * m[2][1] * m[3][3] - m[0][4] * m[1][3] * m[2][1] * m[3][2] + m[0][4] * m[1][3] * m[2][2] * m[3][1];
}

mat4 inverseMat4(mat4 m)
{
	float det = determinant(m);
	if (det == 0.0f)
	{
		// inverse does not exist
		return mat4::identity();
	}

	float invDet = 1.0f / det;
	mat4 res;
	res[0][0] = invDet * (m[1][2] * (m[2][3] * m[3][4] - m[2][4] * m[3][3]) + m[1][3] * (m[2][4] * m[3][2] - m[2][2] * m[3][4]) + m[1][4] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]));
	res[0][1] = -invDet * (m[0][2] * (m[2][3] * m[3][4] - m[2][4] * m[3][3]) + m[0][3] * (m[2][4] * m[3][2] - m[2][2] * m[3][4]) + m[0][4] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]));
	res[0][2] = invDet * (m[0][2] * (m[1][3] * m[3][4] - m[1][4] * m[3][3]) + m[0][3] * (m[1][4] * m[3][2] - m[1][2] * m[3][4]) + m[0][4] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]));
	res[0][3] = -invDet * (m[0][2] * (m[1][3] * m[2][4] - m[1][4] * m[2][3]) + m[0][3] * (m[1][4] * m[2][2] - m[1][2] * m[2][4]) + m[0][4] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]));
	res[1][0] = -invDet * (m[1][1] * (m[2][3] * m[3][4] - m[2][4] * m[3][3]) + m[1][3] * (m[2][4] * m[3][1] - m[2][1] * m[3][4]) + m[1][4] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]));
	res[1][1] = invDet * (m[0][1] * (m[2][3] * m[3][4] - m[2][4] * m[3][3]) + m[0][3] * (m[2][4] * m[3][1] - m[2][1] * m[3][4]) + m[0][4] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]));
	res[1][2] = -invDet * (m[0][1] * (m[1][3] * m[3][4] - m[1][4] * m[3][3]) + m[0][3] * (m[1][4] * m[3][1] - m[1][1] * m[3][4]) + m[0][4] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]));
	res[1][3] = invDet * (m[0][1] * (m[1][3] * m[2][4] - m[1][4] * m[2][3]) + m[0][3] * (m[1][4] * m[2][1] - m[1][1] * m[2][4]) + m[0][4] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]));
	res[2][0] = invDet * (m[1][1] * (m[2][2] * m[3][4] - m[2][4] * m[3][2]) + m[1][2] * (m[2][4] * m[3][1] - m[2][1] * m[3][4]) + m[1][4] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
	res[2][1] = -invDet * (m[0][1] * (m[2][2] * m[3][4] - m[2][4] * m[3][2]) + m[0][2] * (m[2][4] * m[3][1] - m[2][1] * m[3][4]) + m[0][4] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
	res[2][2] = invDet * (m[0][1] * (m[1][2] * m[3][4] - m[1][4] * m[3][2]) + m[0][2] * (m[1][4] * m[3][1] - m[1][1] * m[3][4]) + m[0][4] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]));
	res[2][3] = -invDet * (m[0][1] * (m[1][2] * m[2][4] - m[1][4] * m[2][2]) + m[0][2] * (m[1][4] * m[2][1] - m[1][1] * m[2][4]) + m[0][4] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]));
	res[3][0] = -invDet * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
	res[3][1] = invDet * (m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[0][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
	res[3][2] = -invDet * (m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) + m[0][2] * (m[1][3] * m[3][1] - m[1][1] * m[3][3]) + m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]));
	res[3][3] = invDet * (m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) + m[0][2] * (m[1][3] * m[2][1] - m[1][1] * m[2][3]) + m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]));
	return res;
}

// a recursive function to read all bones and form skeleton
bool readSkeleton(Bone& boneOutput, aiNode* node, unordered_map<string, pair<int, mat4>>* boneInfoTable)
{
	// check if the node is bone
	if (boneInfoTable->find(node->mName.C_Str()) != boneInfoTable->end())
	{
		boneOutput.name.assign(node->mName.C_Str());
		boneOutput.id = (*boneInfoTable)[boneOutput.name].first;
		boneOutput.offset = (*boneInfoTable)[boneOutput.name].second;

		for (uint i = 0; i < node->mNumChildren; i++)
		{
			Bone child;
			readSkeleton(child, node->mChildren[i], boneInfoTable);
			boneOutput.child.push_back(child);
		}
		return true;
	}
	else // find bones in childer
	{
		for (uint i = 0; i < node->mNumChildren; i++)
		{
			if (readSkeleton(boneOutput, node->mChildren[i], boneInfoTable))
			{
				return true;
			}
		}
	}
	return false;
}

void loadModel(Model* pModel, const aiScene* scene, aiMesh* mesh)
{
	vector<Vertex> vertices = {};
	vector<uint> indices = {};

	// load position, normal, texcoord
	for (uint i = 0; i < mesh->mNumVertices; i++)
	{
		// process position
		Vertex vertex;
		vec3 vec;
		vec[0] = mesh->mVertices[i][0];
		vec[1] = mesh->mVertices[i][1];
		vec[2] = mesh->mVertices[i][2];
		vertex.position = vec;

		// process normal
		vec[0] = mesh->mNormals[i][0];
		vec[1] = mesh->mNormals[i][1];
		vec[2] = mesh->mNormals[i][2];
		vertex.normal = vec;

		// process uv
		if (mesh->HasTextureCoords(0))
		{
			vec2 v;
			v[0] = mesh->mTextureCoords[0][i][0];
			v[1] = mesh->mTextureCoords[0][i][1];
			vertex.uv = v;
		}

		vertex.boneIDs = ivec4(0, 0, 0, 0);
		vertex.boneWeights = vec4(0.0f, 0.0f, 0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// load indices
	for (uint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for (uint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	pModel->idxCount = (GLsizei)indices.size();

	// if model is animated, load boneData to vertices
	if (pModel->isAnimated)
	{
		unordered_map<string, pair<int, mat4>> boneInfo = {};
		vector<uint> boneCounts;
		boneCounts.resize(vertices.size(), 0);
		pModel->boneCount = mesh->mNumBones;

		// loop through each bone
		for (uint i = 0; i < pModel->boneCount; i++)
		{
			aiBone* bone = mesh->mBones[i];
			mat4 m = assimpTomat4(bone->mOffsetMatrix);
			boneInfo[bone->mName.C_Str()] = { i, m };

			// loop through each vertex that have that bone
			for (uint j = 0; j < bone->mNumWeights; j++)
			{
				uint id = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;
				boneCounts[id]++;

				switch (boneCounts[id])
				{
				case 1:
					vertices[id].boneIDs[0] = i;
					vertices[id].boneWeights[0] = weight;
					break;

				case 2:
					vertices[id].boneIDs[1] = i;
					vertices[id].boneWeights[1] = weight;
					break;

				case 3:
					vertices[id].boneIDs[2] = i;
					vertices[id].boneWeights[2] = weight;
					break;

				case 4:
					vertices[id].boneIDs[3] = i;
					vertices[id].boneWeights[3] = weight;
					break;

				default:
					break;
				}
			}
		}

		// normalize weights to make all weights sum 1
		for (uint i = 0; i < vertices.size(); i++)
		{
			vec4& boneWeights = vertices[i].boneWeights;
			float totalWeight = boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3];
			if (totalWeight > 0.0f)
			{
				vertices[i].boneWeights[0] /= totalWeight;
				vertices[i].boneWeights[1] /= totalWeight;
				vertices[i].boneWeights[2] /= totalWeight;
				vertices[i].boneWeights[3] /= totalWeight;
			}
		}

		// create bone tree
		readSkeleton(pModel->skeleton, scene->mRootNode, &boneInfo);
	}

	// create vao to store geometry data
	glGenVertexArrays(1, &(pModel->vao));
	glBindVertexArray(pModel->vao);
	{
		// store per vertex data in vbo
		glGenBuffers(1, &(pModel->vbo));
		glBindBuffer(GL_ARRAY_BUFFER, pModel->vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		{
			glVertexAttribPointer(RMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
			glVertexAttribPointer(RMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			glVertexAttribPointer(RMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
			glVertexAttribIPointer(RMC_ATTRIBUTE_BONEIDS, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
			glVertexAttribPointer(RMC_ATTRIBUTE_BONEWEIGHTS, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));

			glEnableVertexAttribArray(RMC_ATTRIBUTE_POSITION);
			glEnableVertexAttribArray(RMC_ATTRIBUTE_NORMAL);
			glEnableVertexAttribArray(RMC_ATTRIBUTE_TEXTURE0);
			glEnableVertexAttribArray(RMC_ATTRIBUTE_BONEIDS);
			glEnableVertexAttribArray(RMC_ATTRIBUTE_BONEWEIGHTS);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// store indexing data in vboIdx
		glGenBuffers(1, &(pModel->vboIdx));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pModel->vboIdx);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), indices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

void loadAnimation(Model* pModel, const aiScene* scene)
{
	// loading first animation
	aiAnimation* anim = scene->mAnimations[0];

	if (anim->mTicksPerSecond != 0.0f)
		pModel->anim->ticksPerSecond = (float)anim->mTicksPerSecond;
	else
		pModel->anim->ticksPerSecond = 1;

	pModel->anim->duration = (float)anim->mDuration * pModel->anim->ticksPerSecond;
	unordered_map<string, BoneTransformTrack> map;
	//pModel->anim->boneTransform.reserve(anim->mNumChannels);

	// load position, rotation and scale for each bone
	// each channel represents each bone
	for (uint i = 0; i < anim->mNumChannels; i++)
	{
		aiNodeAnim* channel = anim->mChannels[i];
		BoneTransformTrack track;

		for (uint j = 0; j < channel->mNumPositionKeys; j++)
		{
			track.positionTimestamps.push_back((float)channel->mPositionKeys[j].mTime);
			track.positions.push_back(assimpTovec3(channel->mPositionKeys[j].mValue));
		}

		for (uint j = 0; j < channel->mNumRotationKeys; j++)
		{
			track.rotationTimestamps.push_back((float)channel->mRotationKeys[j].mTime);
			track.rotations.push_back(assimpTovec4(channel->mRotationKeys[j].mValue));
		}

		for (uint j = 0; j < channel->mNumScalingKeys; j++)
		{
			track.scaleTimestamps.push_back((float)channel->mScalingKeys[j].mTime);
			track.scales.push_back(assimpTovec3(channel->mScalingKeys[j].mValue));
		}

		map[channel->mNodeName.C_Str()] = track;
	}
	pModel->anim->boneTransform = new unordered_map<string, BoneTransformTrack>(map);
}

pair<uint, float> getTimeFraction(vector<float>& times, float& dt)
{
	uint segment = 0;
	while (dt > times[segment])
	{
		segment++;

		// if the animation is longer than last keyframe just render the last 
		// keyframe for rest of the animation time
		if (segment == times.size())
		{
			segment--;
			// frac is hardcoded to 1.0 as we have already passed the last
			// timestamp of last keyframe, so clamping to 1.0 will avoid
			// unnecessary extrapolation
			return { segment, 1.0f };
		}
	}

	segment = segment > 0 ? segment : 1;

	float start = times[segment - 1];
	float end = times[segment];
	float frac = (dt - start) / (end - start);
	return { segment, frac };
}

void getPose(Animation& animation, Bone& skeleton, float dt, vector<mat4>& output, mat4& parentTransform, mat4& globalInverseTransform)
{
	BoneTransformTrack& btt = (*(animation.boneTransform))[skeleton.name];

	// timestamp for which pose is required
	dt = fmod(dt, animation.duration);
	pair<uint, float> fp;
	mat4 globalTransform = parentTransform;

	if (btt.positions.size() != 0)
	{
		// calculate interpolated position
		fp = getTimeFraction(btt.positionTimestamps, dt);
		vec3 position1 = btt.positions[fp.first - 1];
		vec3 position2 = btt.positions[fp.first];
		vec3 position = lerp(position1, position2, fp.second);

		// calculate interpolated rotation
		fp = getTimeFraction(btt.rotationTimestamps, dt);
		vec4 rotation1 = btt.rotations[fp.first - 1];
		vec4 rotation2 = btt.rotations[fp.first];
		vec4 rotation = quatSlerp(rotation1, rotation2, fp.second);

		// calculate interpolated scale
		fp = getTimeFraction(btt.scaleTimestamps, dt);
		vec3 scale1 = btt.scales[fp.first - 1];
		vec3 scale2 = btt.scales[fp.first];
		vec3 scale = lerp(scale1, scale2, fp.second);

		mat4 localTransform = affineTransform(scale, quaternion(rotation), position);

		globalTransform = parentTransform * localTransform;
	}

	output[skeleton.id] = globalInverseTransform * globalTransform * skeleton.offset;

	// update value for child bones
	for (Bone& child : skeleton.child)
	{
		if (!child.name.empty())
			getPose(animation, child, dt, output, globalTransform, globalInverseTransform);
	}
}

Model* LoadModel(const char* path, bool animate)
{
	LogI("loading model: %s [%s]", path, animate ? "with animation" : "without animation");
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LogE("assimp: %s", importer.GetErrorString());
		return NULL;
	}

	LogD("assimp: file parsing done..");

	Model* pModel = new Model();
	pModel->isAnimated = animate;

	aiMesh* mesh = scene->mMeshes[0];

	// inverse the global transformation matrix
	pModel->globalInvTransform = assimpTomat4(scene->mRootNode->mTransformation);
	pModel->globalInvTransform = inverseMat4(pModel->globalInvTransform);

	LogD("assimp: loading model geometry..");
	loadModel(pModel, scene, mesh);
	LogD("assimp: model loading finished..");

	if (pModel->isAnimated)
	{
		pModel->anim = new Animation();

		LogD("assimp: loading animation..");
		loadAnimation(pModel, scene);
		LogD("assimp: animation loading finished..");
	}

	return pModel;
}

void GetPose(Model* pModel, float dt, vector<mat4>& outputPos)
{
	mat4 identity = mat4::identity();
	getPose(*pModel->anim, pModel->skeleton, dt, outputPos, identity, pModel->globalInvTransform);
}

void DrawModel(Model* pModel)
{
	glBindVertexArray(pModel->vao);
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pModel->vboIdx);
		glDrawElements(GL_TRIANGLES, pModel->idxCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

void FreeModel(Model* pModel)
{
	if (pModel)
	{
		glDeleteVertexArrays(1, &(pModel->vao));
		glDeleteBuffers(1, &(pModel->vbo));
		glDeleteBuffers(1, &(pModel->vboIdx));

		if (pModel->anim)
		{
			free(pModel->anim);
		}
	}
}