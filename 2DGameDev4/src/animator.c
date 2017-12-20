#include "animator.h"
#include "simple_logger.h"

typedef struct AnimatorManager_s
{
	HashMap *animatorList;
} AnimatorManager;

AnimatorManager animator_manager;

void animator_system_close()
{
	if (animator_manager.animatorList != NULL)
	{
		hashmap_delete(animator_manager.animatorList, animator_free);
	}
}

void animator_system_init()
{
	animator_manager.animatorList = hashmap_new(16, 4);
	if (!animator_manager.animatorList) 
	{
		slog("Unable to initialize the animator manager!");
		return;
	}
	atexit(animator_system_close);
	slog("Animator system initialized.");
}

Animator *animator_get_by_name(char *name)
{
	Animator *rtn = NULL;
	if (!name) return NULL;
	rtn = hashmap_get_value(name, animator_manager.animatorList);
	return rtn;
}

Animator *animator_new(char *name, Sprite *sprite)
{
	Animator *anim = NULL;
	if (!sprite)
	{
		slog("Cannot intialize an animator without a sprite!");
		return NULL;
	}
	if (!name)
	{
		slog("Cannot initialize an animator without a name!");
		return NULL;
	}
	if (hashmap_get_value(name, animator_manager.animatorList) != NULL)
	{
		slog("An animator with that name already exists!");
		return NULL;
	}
	
	anim = (Animator *)malloc(sizeof(Animator));
	if (!anim) return NULL;

	memset(anim, 0, sizeof(Animator));

	anim->animations = hashmap_new(16, 2);
	if (!anim->animations) 
	{
		free(anim);
		return NULL;
	}
	anim->sprite = sprite;

	animator_manager.animatorList = hashmap_insert(name, anim, animator_manager.animatorList, hash);
	return anim;
}

void animator_free(Animator *anim)
{
	if (!anim) return NULL;
	if (anim->animations != NULL)
	{
		hashmap_delete(anim->animations, animation_free);
	}
	if (anim->sprite != NULL)
		gf2d_sprite_free(anim->sprite);
	free(anim);
}

Animation *animation_new(char *animatorName, char *animationName, Uint32 startFrame, Uint32 endFrame, int loops, char *next, void(*trigger)(), Uint32 triggerFrame)
{
	Animation *animation = NULL;
	if (!animationName)
	{
		slog("Cannot create an animation with no name!");
		return NULL;
	}
	if (!animatorName)
	{
		slog("Cannot insert this animation into an unnamed animator!");
		return NULL;
	}
	else if (hashmap_get_value(animatorName, animator_manager.animatorList) == NULL)
	{
		slog("There is no animator by that name! Cannot insert this animation.");
		return NULL;
	}
	else if (((Animator *)hashmap_get_value(animatorName, animator_manager.animatorList))->animations == NULL)
	{
		slog("There is no animation list within the animator! Cannot insert this animation");
		return NULL;
	}
	animation = (Animation *)malloc(sizeof(Animation));
	if (!animation)
	{
		slog("Failed to allocate memory for a new animation!");
		return NULL;
	}
	memset(animation, 0, sizeof(Animation));

	animation->startFrame = startFrame;
	animation->endFrame = endFrame;
	animation->loops = loops;
	strcpy(animation->name, animationName);
	if (next != NULL)
		strcpy(animation->next, next);
	animation->trigger = trigger;
	animation->triggerFrame = triggerFrame;

	((Animator *)hashmap_get_value(animatorName, animator_manager.animatorList))->animations = hashmap_insert(animation->name, animation, ((Animator *)hashmap_get_value(animatorName, animator_manager.animatorList))->animations, hash);

	return animation;
}

void animation_free(Animation *animation)
{
	if (!animation) return NULL;
	free(animation);
}