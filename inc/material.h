#pragma once

#include "main.h"
#include "logger.h"

// struct to hold material textures
typedef struct _material
{
	// albedoMap;
	// normalMap;
	// metallicMap;
	// roughnessMap;
	// aoMap;
	GLuint maps[5];

} Material;


Material *loadMaterial(const char*);
void useMaterial(Material*);
void deleteMaterial(Material*);

