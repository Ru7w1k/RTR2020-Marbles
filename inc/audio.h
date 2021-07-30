#pragma once
#include "main.h"

bool InitOpenAL(void);
ALuint LoadAudio(const char* path);
void PlayAudio(ALuint buffer);
void UnloadAudio(ALuint buffer);
void UninitOpenAL(void);
