// headers
#include "main.h"
#include "primitives.h"

#include <vector>
using namespace std;

// code
void DrawSphere(void)
{
	// static variables
	static GLuint vao = 0;
	static GLuint vbo = 0;
	static GLuint ebo = 0;
	static int iNoOfCoords = 0;
	static int iNoOfElements = 0;

	// variables
	const GLfloat r = 1.0f;
	const int n = 60;
	int i, j;
	GLfloat phi1, phi2, theta, s, t;
	GLfloat ex, ey, ez, px, py, pz;

	// code
	if (!vao)
	{
		vector<vec3> positions;
		vector<vec3> normals;
		vector<vec2> uv;
		vector<unsigned int> indices;

		for (j = 0; j <= n; j++) {
			for (i = 0; i <= n; i++) {
				float xSeg = (float)i / (float)n;
				float ySeg = (float)j / (float)n;

				float xPos = cosf(xSeg * 2.0f * M_PI) * sinf(ySeg * M_PI);
				float yPos = cosf(ySeg * M_PI);
				float zPos = sinf(xSeg * 2.0f * M_PI) * sinf(ySeg * M_PI);

				positions.push_back(r * vec3(xPos, yPos, zPos));
				normals.push_back(vec3(xPos, yPos, zPos));
				uv.push_back(vec2(xSeg, ySeg));
			}
		}

		bool bOddRow = false;
		for (int y = 0; y < n; y++)
		{
			if (!bOddRow)
			{
				for (int x = 0; x <= n; x++)
				{
					indices.push_back(y * (n + 1) + x);
					indices.push_back((y + 1) * (n + 1) + x);
				}
			}
			else
			{
				for (int x = n; x >= 0; x--)
				{
					indices.push_back((y + 1) * (n + 1) + x);
					indices.push_back(y * (n + 1) + x);
				}
			}
			bOddRow = !bOddRow;
		}
		iNoOfElements = indices.size();

		vector<float> vertData;
		for (int i = 0; i < positions.size(); i++)
		{
			vertData.push_back(positions[i][0]);
			vertData.push_back(positions[i][1]);
			vertData.push_back(positions[i][2]);
			
			vertData.push_back(normals[i][0]);
			vertData.push_back(normals[i][1]);
			vertData.push_back(normals[i][2]);
			
			vertData.push_back(uv[i][0]);
			vertData.push_back(uv[i][1]);
		}

		// create vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create vbo
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertData.size() * sizeof(float), vertData.data(), GL_STATIC_DRAW);

		// vertex position
		glVertexAttribPointer(RMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_POSITION);

		// vertex normals
		glVertexAttribPointer(RMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_NORMAL);

		// vertex texcoords
		glVertexAttribPointer(RMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// create ebo
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLE_STRIP, iNoOfElements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawCube(void)
{
	static GLuint vao = 0;
	static GLuint vbo = 0;

	// vertex array
	const GLfloat cubeData[] = {
		/* Top */
		 0.4f,  0.4f, -0.4f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		-0.4f,  0.4f, -0.4f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-0.4f,  0.4f,  0.4f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		 0.4f,  0.4f,  0.4f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,

		 /* Bottom */
		  0.4f, -0.4f,  0.4f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		 -0.4f, -0.4f,  0.4f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,
		 -0.4f, -0.4f, -0.4f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		  0.4f, -0.4f, -0.4f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,

		  /* Front */
		   0.4f,  0.4f,  0.4f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		  -0.4f,  0.4f,  0.4f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
		  -0.4f, -0.4f,  0.4f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		   0.4f, -0.4f,  0.4f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,

		   /* Back */
			0.4f, -0.4f, -0.4f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		   -0.4f, -0.4f, -0.4f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		   -0.4f,  0.4f, -0.4f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
			0.4f,  0.4f, -0.4f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f,

			/* Right */
			0.4f,  0.4f, -0.4f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
			0.4f,  0.4f,  0.4f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
			0.4f, -0.4f,  0.4f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
			0.4f, -0.4f, -0.4f,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,

			/* Left */
			-0.4f,  0.4f,  0.4f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
			-0.4f,  0.4f, -0.4f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
			-0.4f, -0.4f, -0.4f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
			-0.4f, -0.4f,  0.4f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f
	};

	if (!vao || !vbo)
	{
		// create vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create vbo
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

		// vertex position
		glVertexAttribPointer(RMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_POSITION);

		// vertex normals
		glVertexAttribPointer(RMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_NORMAL);

		// vertex texcoords
		glVertexAttribPointer(RMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_TEXTURE0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);
}

void DrawPlane(void)
{
	static GLuint vao = 0;
	static GLuint vbo = 0;

	// vertex array
	const GLfloat cubeData[] = {
		 1.0f,  1.0f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
	};

	if (!vao || !vbo)
	{
		// create vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create vbo
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

		// vertex position
		glVertexAttribPointer(RMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_POSITION);

		// vertex normals
		glVertexAttribPointer(RMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_NORMAL);

		// vertex texcoords
		glVertexAttribPointer(RMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(RMC_ATTRIBUTE_TEXTURE0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}
