//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
//-------------------------------------------------------------------------

/*
 * A reimplementation of Jim Dose's FX_MAN routines, using  SDL_mixer 1.2.
 *   Whee. FX_MAN is also known as the "Apogee Sound System", or "ASS" for
 *   short. How strangely appropriate that seems.
 */

#include <stdio.h>
#include <errno.h>

#include "duke3d.h"
#include "cache1d.h"

#define _NEED_SDLMIXER	1
//#include "sdl_inc.h"
//#include "fluidsynth_inc.h"
#include "midi.h"
#include "music.h"

#include "timidity.h"
#include "timidity_internal.h"
#include "jaudiolib/src/multivoc.h"
#include "jaudiolib/src/_multivc.h"

enum MidiState 
{
	MIDI_STOPPED = 0,
	MIDI_PLAYING = 1,
};

static struct {
	MidIStream *stream;
	MidSongOptions options;
	MidSong *song;

	int status;
	int looping;
	uint32 song_length;
	uint32 song_position;
} _midi; ///< Metadata about the midi we're playing.

int32_t MUSIC_ErrorCode = MUSIC_Ok;
static int32_t music_initialized = 0;
int32_t music_handle = 0;
int32_t MusicMaxVolume = 255;

static int16_t MusicVolumeTable[255 + 1][256];

char *MUSIC_ErrorString(int32_t ErrorNumber)
{
	switch (ErrorNumber)
	{
	default:
		return "Unknown error.";
	}

	return NULL;
}

#define BUFFERSIZE 2048
void update_audio(char **ptr, uint32_t *length)
{
	static char buffer[BUFFERSIZE];
	*ptr = buffer;

	memset(buffer, 0, BUFFERSIZE);

	if (_midi.status == MIDI_PLAYING)
	{
		*length = mid_song_read_wave(_midi.song, buffer, BUFFERSIZE);

		// If the song has finished, loop it
		if (*length < BUFFERSIZE && _midi.looping == MUSIC_LoopSong && _midi.song->current_sample >= _midi.song->samples)
		{
			int remainder = BUFFERSIZE - *length;
			mid_song_start(_midi.song);
			*length += mid_song_read_wave(_midi.song, buffer + *length, remainder);
		}
	}
	else
	{
		*length = 0;
	}
}

int32_t MUSIC_Init(int32_t SoundCard, int32_t Address)
{
	if (getenv("EDUKE32_MUSIC_CMD")) {
		initprintf("External MIDI player not supported by Timidity backend");
	}

	if (music_initialized) 
	{
		printf("Music already intialized!");
		return MUSIC_Ok;
	}
	
	// Figure out if any of the timidity files exist
	static const char* configFiles[] = { "/opk/timidity.cfg", "/mnt/FunKey/.eduke32/timidity.cfg" };

	FILE *fp;
	int32_t i;

	for (i = (sizeof(configFiles) / sizeof(configFiles[0])) - 1; i >= 0; i--)
	{
		fp = Bfopen(configFiles[i], "r");
		if (fp == NULL)
		{
			if (i == 0)
			{
				printf("Error: couldn't open any of the following files:\n");
				for (i = (sizeof(configFiles) / sizeof(configFiles[0])) - 1; i >= 0; i--)
				{
					initprintf("%s\n", configFiles[i]);
				}

				return(MUSIC_Error);
			}
			continue;
		}
		else
		{
			printf("Using Timidity config file: %s\n", configFiles[i]);
			break;
		}
	}
	Bfclose(fp);

	int result = mid_init(configFiles[i]);
	if (result < 0)
	{
		printf("Error initializing Timidity: %d\n", result);
		return MUSIC_Error;
	}

	_midi.status = MIDI_STOPPED;
	_midi.song = NULL;
	_midi.looping = 0;

	_midi.options.rate = ud.config.MixRate;
	_midi.options.format = MID_AUDIO_U8;
	_midi.options.channels = 1;
	_midi.options.width = ud.config.NumBits == 16 ? 2 : 1;
	_midi.options.buffer_size = _midi.options.rate;

	music_handle = FX_StartDemandFeedPlayback(update_audio, ud.config.MixRate, 0, 255, 255, 255, FX_MUSIC_PRIORITY, 0);
	if (music_handle == FX_Warning)
	{
		printf("FX_StartDemandFeedPlayback failed\n");
		return MUSIC_Error;
	}

	MUSIC_SetVolume(ud.config.MusicVolume);

	// HACK: We're using an MV_Voice to render the music to, which are affected by the SFX Volume. We want to keep music volume independent from the SFX,
	// so I've duplicated the code from MV_SetVoiceVolume and MV_GetVolumeTable.
	VoiceNode* voice = MV_GetVoice(music_handle);
	if (voice)
	{
		int32_t volume = MIX_VOLUME(MusicMaxVolume);
		int16_t *table = (int16_t *)&MusicVolumeTable[volume];

		voice->LeftVolume = table;
		voice->RightVolume = table;

		MV_SetVoiceMixMode(voice);
	}

	MUSIC_SetLoopFlag(MUSIC_LoopSong); // Loop by default

	music_initialized = 1;

	return MUSIC_Ok;
}

void MUSIC_CalcVolume(int32_t MaxVolume)
{
	// For each volume level, create a translation table with the
	// appropriate volume calculated.
	for (int32_t volume = 0; volume <= MV_MaxVolume; volume++)
	{
		MV_CreateVolumeTable(volume, volume, MaxVolume, MusicVolumeTable);
	}
}

int32_t MUSIC_Shutdown(void)
{
	if (music_initialized)
	{
		if (music_handle > 0)
		{
			FX_StopSound(music_handle);
			music_handle = 0;
		}

		printf("Shutting down Timidity\n");
		mid_exit();

		music_initialized = 0;
	}

	return MUSIC_Ok;
}

void MUSIC_SetVolume(int32_t volume)
{
	volume = max(0, volume);
	volume = min(volume, MV_MaxTotalVolume);

	MusicMaxVolume = volume;

	MUSIC_CalcVolume(volume);
}

int32_t MUSIC_GetVolume(void)
{
	return MusicMaxVolume;
}

void MUSIC_SetLoopFlag(int32_t loopflag)
{
	_midi.looping = loopflag;
}

void MUSIC_Continue(void)
{
	if (music_handle > 0)
	{
		FX_PauseVoice(music_handle, 0);
		_midi.status = MIDI_PLAYING;
	}
}

void MUSIC_Pause(void)
{
	if (music_handle > 0)
	{
		FX_PauseVoice(music_handle, 1);
		_midi.status = MIDI_STOPPED;
	}
}

int32_t MUSIC_StopSong(void)
{
	if (!music_initialized)
	{
		return MUSIC_Error;
	}

	MUSIC_Pause();
	
	if (_midi.song != NULL)
	{
		mid_song_free(_midi.song);
		_midi.song = NULL;
	}
	
	return MUSIC_Ok;
}

int32_t MUSIC_PlaySong(char *song, int32_t loopflag)
{
	if (!music_initialized)
	{
		return MUSIC_Error;
	}

	MUSIC_StopSong();

	_midi.stream = mid_istream_open_mem(song, g_musicSize, 0);
	if (_midi.stream == NULL) 
	{
		printf("Could not open music data");
		return MUSIC_Error;
	}

	_midi.song = mid_song_load(_midi.stream, &_midi.options);
	mid_istream_close(_midi.stream);
	_midi.song_length = mid_song_get_total_time(_midi.song);

	if (_midi.song == NULL) 
	{
		printf("Invalid MIDI file");
		return MUSIC_Error;
	}

	mid_song_start(_midi.song);
	MUSIC_Continue();

	return MUSIC_Ok;
}

int32_t MUSIC_InitMidi(int32_t card, midifuncs *Funcs, int32_t Address)
{
	return MIDI_Ok;
}

void MUSIC_Update(void) 
{

}
