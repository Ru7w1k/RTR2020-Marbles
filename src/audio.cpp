// headers 
#include "main.h"
#include "logger.h"

#include "audio.h"

#include <iostream>
#include <fstream>
#include <cstring>

//// OpenAL state ////////////////////////

static ALCdevice* device = NULL;
static ALCcontext* context = NULL;

//////////////////////////////////////////


// wav file helper
bool isBigEndian()
{
    int a = 1;
    return !((char*)&a)[0];
}

int convertToInt(char* buffer, int len)
{
    int a = 0;
    if (!isBigEndian())
    {
        for (int i = 0; i < len; i++)
            ((char*)&a)[i] = buffer[i];
    }
    else
    {
        for (int i = 0; i < len; i++)
            ((char*)&a)[3 - i] = buffer[i];
    }
    return a;
}

char* loadWav(const char* fn, int* chan, int* samplerate, int* bps, int* size)
{
    char buffer[4];
    std::ifstream in(fn, std::ios::binary);
    in.read(buffer, 4);
    if (strncmp(buffer, "RIFF", 4) != 0)
    {
        LogE("%s is not a valid WAVE file..", fn);
        return NULL;
    }

    in.read(buffer, 4);
    in.read(buffer, 4);  // WAVE
    in.read(buffer, 4);  // fmt
    in.read(buffer, 4);  // 16
    in.read(buffer, 2);  // 1

    in.read(buffer, 2);
    *chan = convertToInt(buffer, 2);

    in.read(buffer, 4);
    *samplerate = convertToInt(buffer, 4);

    in.read(buffer, 4);
    in.read(buffer, 2);

    in.read(buffer, 2);
    *bps = convertToInt(buffer, 2);

    in.read(buffer, 4);

    in.read(buffer, 4);
    *size = convertToInt(buffer, 4);

    char* data = new char[*size];
    in.read(data, *size);
    return data;
}


// Public Functions
bool InitOpenAL(void)
{
	// select the preferred device
	device = alcOpenDevice(NULL);
	if (!device)
	{
		LogE("OpenAL: Failed to open device");
		return false;
	}
	LogD("OpenAL: Device created");
	
	// create context for device
	context = alcCreateContext(device, NULL);
	if (!context)
	{
		LogE("OpenAL: Failed to create context");
		return false;
	}
	LogD("OpenAL: Context created");

	if (!alcMakeContextCurrent(context))
	{
		LogE("OpenAL: Failed to set context");
		return false;
	}
	LogD("OpenAL: Context set");

	ALenum err = alGetError(); // clear the error

	return true;
}

ALuint LoadAudio(const char* path)
{
	ALuint buffer;
	ALuint source;
	
	ALenum alformat;
	ALvoid* aldata;
	int chan, samplerate, bps, size;
	
	alGenBuffers(1, &buffer);

	aldata = loadWav(path, &chan, &samplerate, &bps, &size);
	if (!aldata)
	{
		LogE("OpenAL: Failed to load audio %s", path);
	}
	LogD("OpenAL: Audio loaded %s", path);

	if (chan == 1)
	{
		alformat = bps == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	}
	else
	{
		alformat = bps == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}

	alBufferData(buffer, alformat, aldata, size, samplerate);

	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);

	return source;
}

void PlayAudio(ALuint source)
{
	alSourcePlay(source);
}

void UnloadAudio(ALuint buffer)
{
	// alDeleteBuffers(1, &buffer);
}

void UninitOpenAL(void)
{
	context = alcGetCurrentContext();
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}


