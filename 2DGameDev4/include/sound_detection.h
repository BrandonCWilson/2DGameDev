#ifndef _SOUND_DETECTION_H_
#define _SOUND_DETECTION_H_

#include "gf2d_vector.h"
#include <SDL.h>

typedef struct Sonar_s
{
	bool inUse;

	Vector2D center;
	double radius;
	double decrement;
	double noise;

	int timer;
} Sonar;

void sonar_init(Uint32 max);

Sonar *sonar_new();

void sonar_draw_all();

void sonar_update();

void sonar_free(Sonar *sonar);

#endif