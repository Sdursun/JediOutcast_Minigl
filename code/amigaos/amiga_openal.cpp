/*
 * OpenAL stand-ins for AmigaOS 3.x.
 *
 * The sound code (snd_dma.cpp) has an OpenAL path next to its software
 * mixer and calls OpenAL directly, so the program needs these symbols to
 * link. There is no OpenAL here: the device cannot be opened, so S_Init
 * stays on the software mixer (AHI, amiga_snd.cpp), and nothing below
 * ever does any work.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "../client/snd_local.h"

ALCdevice *alcOpenDevice( ALCubyte *deviceName ) { return NULL; }
ALCvoid alcCloseDevice( ALCdevice *device ) {}
ALCcontext *alcCreateContext( ALCdevice *device, ALCint *attrList ) { return NULL; }
ALCvoid alcDestroyContext( ALCcontext *context ) {}
ALCboolean alcMakeContextCurrent( ALCcontext *context ) { return ALC_FALSE; }
ALCcontext *alcGetCurrentContext( ALCvoid ) { return NULL; }
ALCdevice *alcGetContextsDevice( ALCcontext *context ) { return NULL; }
ALCenum alcGetError( ALCdevice *device ) { return ALC_INVALID_DEVICE; }

ALenum alGetError( ALvoid ) { return AL_INVALID_OPERATION; }
ALboolean alIsExtensionPresent( const ALubyte *extName ) { return AL_FALSE; }
ALvoid *alGetProcAddress( const ALubyte *fname ) { return NULL; }

ALvoid alGenBuffers( ALsizei n, ALuint *buffers ) {}
ALvoid alDeleteBuffers( ALsizei n, ALuint *buffers ) {}
ALvoid alBufferData( ALuint buffer, ALenum format, ALvoid *data, ALsizei size, ALsizei freq ) {}
ALvoid alGetBufferi( ALuint buffer, ALenum pname, ALint *value ) { *value = 0; }

ALvoid alGenSources( ALsizei n, ALuint *sources ) {}
ALvoid alDeleteSources( ALsizei n, ALuint *sources ) {}
ALvoid alSourcei( ALuint source, ALenum pname, ALint value ) {}
ALvoid alSourcef( ALuint source, ALenum pname, ALfloat value ) {}
ALvoid alSourcefv( ALuint source, ALenum pname, ALfloat *values ) {}
ALvoid alGetSourcei( ALuint source, ALenum pname, ALint *value ) { *value = 0; }
ALvoid alSourcePlay( ALuint source ) {}
ALvoid alSourceStop( ALuint source ) {}
ALvoid alSourceQueueBuffers( ALuint source, ALsizei n, ALuint *buffers ) {}
ALvoid alSourceUnqueueBuffers( ALuint source, ALsizei n, ALuint *buffers ) {}

ALvoid alListenerf( ALenum pname, ALfloat value ) {}
ALvoid alListenerfv( ALenum pname, ALfloat *values ) {}
