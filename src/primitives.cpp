// headers
#include "main.h"
#include "primitives.h"

// code
void DrawSphere(void)
{
	// static variables
	static GLuint vao = 0;
	static GLuint vbo = 0;
	static int iNoOfCoords = 0;

	// variables
	GLfloat r = 1.0f;
	int n = 100;
	int i, j;
	GLfloat phi1, phi2, theta, s, t;
	GLfloat ex, ey, ez, px, py, pz;

	// code
	if (!vao || !vbo)
	{
		GLfloat* vert = (GLfloat*)malloc(8 * sizeof(GLfloat) * n * (n + 1) * 2);

		if (vert)
		{
			for (j = 0; j < n; j++) {
				phi1 = j * (float)M_PI * 2 / n;
				phi2 = (j + 1) * (float)M_PI * 2 / n;

				for (i = 0; i <= n; i++) {
					theta = i * (float)M_PI / n;

					ex = sinf(theta) * cosf(phi2);
					ey = sinf(theta) * sinf(phi2);
					ez = cosf(theta);
					px = r * ex;
					py = r * ey;
					pz = r * ez;

					//vec3 n = -normalize(vec3(ex, ey, ez));
					vert[(iNoOfCoords * 8) + 3] = ex;
					vert[(iNoOfCoords * 8) + 4] = ey;
					vert[(iNoOfCoords * 8) + 5] = ez;
					/*vert[(iNoOfCoords * 8) + 3] = n[0];
					vert[(iNoOfCoords * 8) + 4] = n[1];
					vert[(iNoOfCoords * 8) + 5] = n[2];*/

					s = phi2 / ((float)M_PI * 2);   // column
					t = 1 - (theta / (float)M_PI);  // row
					vert[(iNoOfCoords * 8) + 6] = s;
					vert[(iNoOfCoords * 8) + 7] = t;

					vert[(iNoOfCoords * 8) + 0] = px;
					vert[(iNoOfCoords * 8) + 1] = py;
					vert[(iNoOfCoords * 8) + 2] = pz;

					ex = sinf(theta) * cosf(phi1);
					ey = sinf(theta) * sinf(phi1);
					ez = cosf(theta);
					px = r * ex;
					py = r * ey;
					pz = r * ez;

					// n = -normalize(vec3(ex, ey, ez));
					vert[(iNoOfCoords * 8) + 11] = ex;
					vert[(iNoOfCoords * 8) + 12] = ey;
					vert[(iNoOfCoords * 8) + 13] = ez;
					/*vert[(iNoOfCoords * 8) + 11] = n[0];
					vert[(iNoOfCoords * 8) + 12] = n[1];
					vert[(iNoOfCoords * 8) + 13] = n[2];*/

					s = phi1 / ((float)M_PI * 2);   // column
					t = 1 - (theta / (float)M_PI);  // row
					vert[(iNoOfCoords * 8) + 14] = s;
					vert[(iNoOfCoords * 8) + 15] = t;

					vert[(iNoOfCoords * 8) + 8] = px;
					vert[(iNoOfCoords * 8) + 9] = py;
					vert[(iNoOfCoords * 8) + 10] = pz;

					iNoOfCoords += 2;
				}
			}
		}

		// create vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create vbo
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, 8 * iNoOfCoords * sizeof(GLfloat), vert, GL_STATIC_DRAW);

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
	glDrawArrays(GL_TRIANGLE_STRIP, 0, iNoOfCoords);
	glBindVertexArray(0);
}

void DrawCube(void)
{
	static GLuint vao = 0;
	static GLuint vbo = 0;

	// vertex array
	const GLfloat cubeData[] = {
		/* Top */
		 1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		-1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,

		/* Bottom */
		 1.0f, -1.0f,  1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		-1.0f, -1.0f,  1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		 1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,

		/* Front */
		 1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		-1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
		-1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,

		/* Back */
		 1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		-1.0f,  1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
		 1.0f,  1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f,

		/* Right */
		1.0f,  1.0f, -1.0f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		1.0f,  1.0f,  1.0f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		1.0f, -1.0f,  1.0f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
		1.0f, -1.0f, -1.0f,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,

		/* Left */
		-1.0f,  1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		-1.0f, -1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f
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
		/* Top */
		 1.0f,  0.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		-1.0f,  0.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-1.0f,  0.0f,  1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		 1.0f,  0.0f,  1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
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
