#pragma once
#include "main.h"

#define CAM_FOREWARD  1
#define CAM_BACKWARD  2
#define CAM_LEFT      3
#define CAM_RIGHT     4

#define CAMERA_SPEED       1.0f
#define CAMERA_SENSITIVITY 0.25f
#define CAMERA_ZOOM        10.0f

typedef struct _camera
{
	// camara properties
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;

	float Height;

	// euler angles
	float Yaw;
	float Pitch;

	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

} Camera;

Camera* AddNewCamera(vec3 pos, vec3 front, vec3 up, float yaw, float pitch, float zoom = CAMERA_ZOOM, float height = 0.0f);

void UpdateCameraVectors(Camera* cam);
mat4 GetViewMatrix(Camera *cam);
mat4 GetViewMatrixNoTranslate(Camera* cam);
vec3 GetCameraLookPoint(Camera* cam);
void ProcessKeyboard(Camera* cam, int dir);
void MoveDir(Camera* cam, int dir, float step);
void ProcessMouse(Camera* cam, float xoff, float yoff);
void Zoom(Camera* cam, float z);
void Print(Camera* cam);
void DeleteCamera(Camera* cam);
