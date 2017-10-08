#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <SDL.h>
#include <SDL_mixer.h>
#include "gf2d_text.h"

void audio_init(Uint32 maxSounds, Uint32 channels, Uint32 channelGroups, Uint32 maxMusic, bool enableMP3, bool enableOgg);

typedef struct
{
	Uint32 ref_count;
	TextLine filepath;  /**<the sound file that was loaded*/
	Mix_Chunk *sound;
	float volume;
	int defaultChannel;
}Sound;

Sound *sound_load(char *filename, float volume, int defaultChannel);

void sound_play(Sound *sound, int loops, float volume, int channel, int group);
#endif