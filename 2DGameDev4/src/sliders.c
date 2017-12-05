#include "sliders.h"
#include "audio.h"

void slider_set_master_volume(int volume)
{
	audio_set_master_volume(volume);
}

void slider_set_music_volume(int volume)
{
	audio_set_music_volume(volume);
}

void slider_set_sound_effect_volume(int volume)
{
	audio_set_sound_volume(volume);
}