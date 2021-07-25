#pragma once
#include "camera.h"

// callback function types for scene
typedef bool (*pfInit)   (void);
typedef void (*pfUninit) (void);
typedef void (*pfReset)  (void);

typedef void (*pfDisplay) (void);
typedef bool (*pfUpdate)  (float);
typedef void (*pfResize)  (int, int);

// struct to hold complete scene
typedef struct _scene
{
	char      Name[100];
	pfInit    InitFunc;
	pfUninit  UninitFunc;
	pfDisplay DisplayFunc;
	pfUpdate  UpdateFunc;
	pfResize  ResizeFunc;
	pfReset   ResetFunc;

	Camera* Camera;// For now 1 camera per scene

} Scene;

// functions on scene

// initialize internal queue state for holding scenes
bool InitSceneQueue();

// add a scene into queue
// InitSceneQueue() must be called before this
bool AddScene(Scene*);

// remove the front scene from queue
bool RemoveScene();

// get the front scene from queue without deleting it
bool GetScene(Scene&);
bool GetSceneAt(Scene&, int);

// get the total scene count
int GetSceneCount();

// scene traverse
void PrevScene();
void NextScene();
void SetScene(int);

/* TODO
 
// scene lifecycle
Scene* newScene(char*);
void deleteScene(Scene*);

*/
