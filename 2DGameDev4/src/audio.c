#include "audio.h"
#include "simple_logger.h"

float volume_master = 1.0;
float volume_sound = 10.0;
float volume_music = 10.0;

typedef struct
{
	int max_sounds;
	Sound *sound_list;
} SoundManager;

SoundManager sound_manager;

void audio_close()
{
	slog("audio system closed");
}

void audio_set_master_volume(float volume)
{
	volume_master = volume;
}

void audio_set_music_volume(float volume)
{
	volume_music = volume;
}

void audio_set_sound_volume(float volume)
{
	volume_sound = volume;
}

float audio_get_master_volume()
{
	return volume_master;
}

float audio_get_music_volume()
{
	return volume_music;
}

float audio_get_sound_volume()
{
	return volume_sound;
}

void sound_delete(Sound *sound)
{
	if (!sound)return;
	if (sound->sound != NULL)
	{
		Mix_FreeChunk(sound->sound);
	}
	memset(sound, 0, sizeof(Sound));//clean up all other data
}

void sound_clear_all()
{
	int i;
	for (i = 0; i < sound_manager.max_sounds; i++)
	{
		sound_delete(&sound_manager.sound_list[i]);// clean up the data
	}
}

void sound_close()
{
	sound_clear_all();
	if (sound_manager.sound_list != NULL)
	{
		free(sound_manager.sound_list);
	}
	sound_manager.sound_list = NULL;
	sound_manager.max_sounds = 0;
	slog("sound system closed");
}

void sound_init(Uint32 max)
{
	if (!max)
	{
		slog("cannot intialize a sound manager for Zero sounds!");
		return;
	}
	sound_manager.max_sounds = max;
	sound_manager.sound_list = (Sound *)malloc(sizeof(Sound)*max);
	memset(sound_manager.sound_list, 0, sizeof(Sound)*max);
	slog("sound system initialized");
	atexit(sound_close);
}

void audio_init(Uint32 maxSounds,Uint32 channels,Uint32 channelGroups,Uint32 maxMusic,bool enableMP3,bool enableOgg)
{
	int flags = 0;

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
	{
		slog("Failed to open audio: %s\n", SDL_GetError());
		return;
	}
	atexit(Mix_CloseAudio);

	if (enableMP3)
	{
		flags |= MIX_INIT_MP3;
	}
	if (enableOgg)
	{
		flags |= MIX_INIT_OGG;
	}
	if (!(Mix_Init(flags) & flags))
	{
		slog("failed to initialize some audio support: %s", SDL_GetError());
	}
	atexit(Mix_Quit);
	atexit(audio_close);
	sound_init(maxSounds);
	slog("Audio initialized..");
}


void sound_free(Sound *sound)
{
	if (!sound) return;
	sound->ref_count--;
}

Sound *sound_new()
{
	int i;
	/*search for an unused sound address*/
	for (i = 0; i < sound_manager.max_sounds; i++)
	{
		if ((sound_manager.sound_list[i].ref_count == 0) && (sound_manager.sound_list[i].sound == NULL))
		{
			sound_manager.sound_list[i].ref_count = 1;//set ref count
			return &sound_manager.sound_list[i];//return address of this array element        }
		}
	}
	/*find an unused sound address and clean up the old data*/
	for (i = 0; i < sound_manager.max_sounds; i++)
	{
		if (sound_manager.sound_list[i].ref_count == 0)
		{
			sound_delete(&sound_manager.sound_list[i]);// clean up the old data
			sound_manager.sound_list[i].ref_count = 1;//set ref count
			return &sound_manager.sound_list[i];//return address of this array element
		}
	}
	slog("error: out of sound addresses");
	return NULL;
}

Sound *sound_get_by_filename(char * filename)
{
	int i;
	for (i = 0; i < sound_manager.max_sounds; i++)
	{
		if (gf2d_line_cmp(sound_manager.sound_list[i].filepath, filename) == 0)
		{
			return &sound_manager.sound_list[i];
		}
	}
	return NULL;// not found
}

Sound *sound_load(char *filename, float volume, int defaultChannel)
{
	Sound *sound;
	sound = sound_get_by_filename(filename);
	if (sound)
	{
		sound->ref_count++;
		return sound;
	}
	sound = sound_new();
	if (!sound)
	{
		return NULL;
	}
	sound->sound = Mix_LoadWAV(filename);
	if (!sound->sound)
	{
		slog("failed to load sound file %s", filename);
		sound_free(sound);
		return NULL;
	}
	sound->volume = volume;
	sound->defaultChannel = defaultChannel;
	return sound;
}

void sound_play(Sound *sound, int loops, float volume, int channel, int group, AUDIO_T type)
{
	int chan;
	float netVolume = 1;
	if (!sound)return;
	if (volume > 0)
	{
		netVolume *= volume;
		if ((type == MUSIC_T)&&(audio_get_music_volume() > 0))
			netVolume *= audio_get_music_volume();
		if ((type == SOUND_T)&&(audio_get_sound_volume() > 0))
			netVolume *= audio_get_sound_volume();
		if (audio_get_master_volume > 0)
			netVolume *= audio_get_master_volume();
	}
	if (channel >= 0)
	{
		chan = channel;
	}
	else
	{
		chan = sound->defaultChannel;
	}
	netVolume /= 10000;
	slog("Net Volume: %f", netVolume);
	Mix_VolumeChunk(sound->sound, (int)(netVolume * MIX_MAX_VOLUME));
	Mix_PlayChannel(chan, sound->sound, loops);
}
