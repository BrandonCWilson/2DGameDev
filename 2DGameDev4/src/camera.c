#include "camera.h"

typedef struct
{
	Vector2D position;
} Camera;

Camera main_cam;

Vector2D * camera_get_position()
{
	return &main_cam.position;
}