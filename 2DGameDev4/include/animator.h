#ifndef _ANIMATOR_H_
#define _ANIMATOR_H_

#include <SDL.h>
#include "gf2d_sprite.h"
#include "hashmap.h"

typedef struct Animation_s
{
	char name[128];

	int startFrame;
	int endFrame;

	int loops;

	char next[128];	// the animation to play after this completes

	void(*trigger)();
	int triggerFrame;
} Animation;

typedef struct Animator_s
{
	HashMap *animations;
	char name[128];

	Sprite *sprite;
} Animator;

void animator_system_init();

Animator *animator_new(char *name, Sprite *sprite);

void animator_free(Animator *anim);

Animation *animation_new(char *animatorName, char *animationName, Uint32 startFrame, Uint32 endFrame, int loops, char *next, void(*trigger)(), Uint32 triggerFrame);

void animation_free(Animation *animation);

#endif