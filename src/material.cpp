#include "material.h"
#include "helper.h"


Material* loadMaterial(const char* path)
{
	GLuint loadTexture(const char*);

	char str[256];
	Material *material = (Material*)malloc(sizeof(Material));

	const char* maps[] = {
		"albedo.png\0",
		"normal.png\0",
		"metallic.png\0",
		"roughness.png\0",
		"ao.png\0"
	};

	int l = (int)strlen(path);
	memcpy(str, path, l);
	memcpy(str + l, "\\", 1);
	l++;

	for (int i = 0; i < 5; i++)
	{
		memcpy(str + l, maps[i], strlen(maps[i])+1);
		material->maps[i] = loadTexture(str);
		if (!material->maps[i])
		{
			LogE("%s material failed for texture: %s", path, str);
		}
	}

	return material;
}

void useMaterial(Material* material)
{
	if (!material) return;

	for (int i = 0; i < 5; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, material->maps[i]);
	}
}

void deleteMaterial(Material* material)
{
	if (!material) return;
	
	glDeleteTextures(5, material->maps);
	free(material);
}

