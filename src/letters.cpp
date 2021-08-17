#pragma once

#include "main.h"

#include "model.h"


static Model* letterModels[50];

bool LoadLetters()
{
	char path[256];
	for (char i = '0'; i <= '9'; i++)
	{
		sprintf_s(path, "res\\models\\%c.obj", i);
		letterModels[i - '0'] = LoadModel(path, false);
	}
	for (char i = 'A'; i <= 'Z'; i++)
	{
		sprintf_s(path, "res\\models\\%c.obj", i);
		letterModels[i - 'A' + 10] = LoadModel(path, false);
	}
	return true;
}

void UnloadLetters()
{
	for (int i = 0; i < 50; i++)
	{
		FreeModel(letterModels[i]);
	}
}

Model* GetModel(const char ch)
{
	int idx = 0;
	if ('0' <= ch && ch <= '9')
		idx = ch - '0';

	else if ('A' <= ch && ch <= 'Z')
		idx = ch - 'A' + 10;

	return letterModels[idx];
}
