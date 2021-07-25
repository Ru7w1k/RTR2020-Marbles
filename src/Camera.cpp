// headers
#include "main.h"
#include "logger.h"

#include "Camera.h"

#define CAMERA_SPEED       1.0f;
#define CAMERA_SENSITIVITY 0.25f;
#define CAMERA_ZOOM        10.0f;

Camera* AddNewCamera(vec3 pos, vec3 front, vec3 up, float yaw, float pitch)
{
	Camera* cam = (Camera*)malloc(sizeof(Camera));

	cam->Position = pos;
	cam->Front    = front;
	cam->WorldUp  = up;
	cam->Yaw      = yaw;
	cam->Pitch    = pitch;

	cam->Yaw   = yaw;
	cam->Pitch = pitch;

	cam->MouseSensitivity = CAMERA_SENSITIVITY;
	cam->MovementSpeed    = CAMERA_SPEED;
	cam->Zoom             = CAMERA_ZOOM;

	cam->Height = 0.0f;

	UpdateCameraVectors(cam);

	return cam;
}


void UpdateCameraVectors(Camera* cam)
{
	cam->Position[0] = -40.0f * cosf(radians(cam->Yaw)) * cosf(radians(cam->Pitch));
	cam->Position[1] = -40.0f * sinf(radians(cam->Pitch));
	cam->Position[2] = -40.0f * sinf(radians(cam->Yaw)) * cosf(radians(cam->Pitch));
	cam->Position[1] += cam->Height;

	cam->Front[0] = cosf(radians(cam->Yaw)) * cosf(radians(cam->Pitch));
	cam->Front[1] = sinf(radians(cam->Pitch));
	cam->Front[2] = sinf(radians(cam->Yaw)) * cosf(radians(cam->Pitch));

	cam->Front = normalize(cam->Front);

	// calculate the right and up vectors
	cam->Right = normalize(cross(cam->Front, cam->WorldUp));
	cam->Up    = normalize(cross(cam->Right, cam->Front));
}

mat4 GetViewMatrix(Camera* cam)
{
	return lookat(cam->Position, cam->Position + cam->Front, cam->Up);
}

mat4 GetViewMatrixNoTranslate(Camera* cam)
{
	mat4 m = GetViewMatrix(cam);
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	return m;
}

vec3 GetCameraLookPoint(Camera* cam)
{
	UpdateCameraVectors(cam);
	return cam->Position + cam->Front;
}

void ProcessKeyboard(Camera* cam, int dir) 
{
	MoveDir(cam, dir, 1.0f);
}

void MoveDir(Camera* cam, int dir, float step)
{
	switch (dir)
	{
	case CAM_FOREWARD:
		cam->Height += step;
		break;

	case CAM_BACKWARD:
		cam->Height -= step;
		break;

	case CAM_LEFT:
		cam->Position -= step * cam->Right;
		break;

	case CAM_RIGHT:
		cam->Position += step * cam->Right;
		break;	
	}

	UpdateCameraVectors(cam);
}

void ProcessMouse(Camera* cam, float xoff, float yoff)
{
	xoff *= cam->MouseSensitivity;
	yoff *= cam->MouseSensitivity;

	cam->Yaw   += xoff;
	cam->Pitch -= yoff;

	if (cam->Pitch > 89.0)  cam->Pitch = 89.0;
	if (cam->Pitch < -89.0) cam->Pitch = -89.0;

	UpdateCameraVectors(cam);
}

void Zoom(Camera *cam, float z)
{
	cam->Zoom += z;
}

void Print(Camera* cam)
{
	LogI("----- camera --------------------");
	LogI("Position: %f %f %f", cam->Position[0], cam->Position[1], cam->Position[2]);
	LogI("Front: %f %f %f", cam->Front[0], cam->Front[1], cam->Front[2]);
	LogI("Right: %f %f %f", cam->Right[0], cam->Right[1], cam->Right[2]);
	LogI("Up: %f %f %f", cam->Up[0], cam->Up[1], cam->Up[2]);
	LogI("Yaw: %f", cam->Yaw);
	LogI("Pitch: %f", cam->Pitch);
	LogI("---------------------------------");
}

void DeleteCamera(Camera* cam)
{
	free(cam);
}

