// headers
#include <stdio.h>
#include <stdlib.h>

#include "scene.h"
#include "logger.h"

#define QUEUE_SIZE 25

typedef struct _queue {

    int front;
    int rear;
    int count;
    Scene *data[QUEUE_SIZE];

} Queue;

Queue *SceneQueue = NULL;

bool isEmpty()
{
    return (!SceneQueue || SceneQueue->front == -1 || SceneQueue->front > SceneQueue->rear);
}

bool InitSceneQueue()
{
	// create queue
	SceneQueue = (Queue*)malloc(sizeof(Queue));

    // fail if malloc failed
    if (!SceneQueue) return false;

	SceneQueue->front = -1;
	SceneQueue->rear = -1;
	SceneQueue->count = 0;

	return true;
}

bool AddScene(Scene* scene)
{
    // error conditions
    if (SceneQueue == NULL)
    {
        LogE("Added Scene: %s", scene->Name);
        return false;
    }

    // if it is the first element, set front and rear both
    if (SceneQueue->front == -1 && SceneQueue->rear == -1)
    {
        SceneQueue->front = 0;
        SceneQueue->rear = 0;
    }
    else
    {
        SceneQueue->rear++;
    }

    SceneQueue->data[SceneQueue->rear] = scene;
    SceneQueue->count++;
    LogD("Added Scene: %s", scene->Name);
    return true;
}

bool RemoveScene()
{
    // error conditions
    if (SceneQueue == NULL || isEmpty())
    {
        LogE("Removed Scene failed..");
        return false;
    }

    // soft delete the scene
    // SceneQueue->data[SceneQueue->front] = NULL;
    // SceneQueue->count--;
    SceneQueue->front++;
    LogD("Removed Scene: %s", SceneQueue->data[SceneQueue->front-1]->Name);
    return true;
}

bool GetScene(Scene& scene)
{
    // error conditions
    if (isEmpty()) return false;

    scene = *SceneQueue->data[SceneQueue->front];
    return true;
}

bool GetSceneAt(Scene& scene, int idx)
{
    // error conditions
    if (isEmpty()) return false;
    if (idx >= SceneQueue->count) return false;
    if (!SceneQueue->data[idx]) return false;

    scene = *SceneQueue->data[idx];
    return true;
}

int GetSceneCount()
{
    // error conditions
    if (isEmpty()) return 0;

    return SceneQueue->count;
}

void PrevScene()
{
    // error conditions
    if (isEmpty()) return;

    // if already on first scene, return
    if (SceneQueue->front == 0) return;

    // go to previous scene
    SceneQueue->front--;

    // reset the scene
    SceneQueue->data[SceneQueue->front]->ResetFunc();
}

void NextScene()
{
    // error conditions
    if (isEmpty()) return;

    // if already on last scene, return
    if (SceneQueue->front == SceneQueue->count-1) return;

    // go to next scene
    SceneQueue->front++;

    // reset the scene
    SceneQueue->data[SceneQueue->front]->ResetFunc();
}

void SetScene(int sceneNo)
{
    // error conditions
    if (isEmpty()) return;

    // if sceneNo is out of range [0, count-1]
    if (sceneNo < 0 || sceneNo > SceneQueue->count - 1) return;

    // go to requested scene
    SceneQueue->front = sceneNo;

    // reset the scene
    SceneQueue->data[SceneQueue->front]->ResetFunc();
}
