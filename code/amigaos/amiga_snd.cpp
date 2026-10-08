/*
 * Copyright (C) 2005,2010 Mark Olsen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <exec/exec.h>
#include <utility/hooks.h>
#include <devices/ahi.h>
#include <proto/exec.h>
#define USE_INLINE_STDARG
#include <proto/ahi.h>

#include "../game/q_shared.h"
#include "../client/snd_local.h"

void Amiga_Trace(const char *msg);

struct AHIdata *ad;

struct AHIChannelInfo
{
	struct AHIEffChannelInfo aeci;
	ULONG offset;
};

struct AHIdata
{
	struct MsgPort *msgport;
	struct AHIRequest *ahireq;
	int ahiopen;
	struct AHIAudioCtrl *audioctrl;
	void *samplebuffer;
	struct Hook EffectHook;
	struct AHIChannelInfo aci;
	unsigned int readpos;
};

/* AHI calls this hook from its mixing interrupt with the hook in A0, the
   audio control in A2 and the channel info in A1. */
static ULONG EffectFunc(register struct Hook *hook __asm("a0"), register struct AHIAudioCtrl *actrl __asm("a2"), register struct AHIEffChannelInfo *aeci __asm("a1"))
{
	struct AHIdata *ad;

	ad = hook->h_Data;

	ad->readpos = aeci->ahieci_Offset[0];

	return 0;
}


qboolean SNDDMA_Init(void)
{
	ULONG channels;
	ULONG speed;
	ULONG bits;
	unsigned int buffersize;

	ULONG r;

	struct Library *AHIBase;

	struct AHISampleInfo sample;

	cvar_t *sndbits;
	cvar_t *sndspeed;
	cvar_t *sndchannels;

	char modename[64];

	if (ad)
		return 1;

	sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE);
	sndspeed = Cvar_Get("sndspeed", "0", CVAR_ARCHIVE);
	sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE);

	speed = sndspeed->integer;

	if (speed == 0)
		speed = 22050;

	Amiga_Trace("SNDDMA_Init: start");
	ad = AllocVec(sizeof(*ad), MEMF_ANY);
	if (ad)
	{
		ad->msgport = CreateMsgPort();
		if (ad->msgport)
		{
			ad->ahireq = (struct AHIRequest *)CreateIORequest(ad->msgport, sizeof(struct AHIRequest));
			if (ad->ahireq)
			{
				Amiga_Trace("SNDDMA_Init: OpenDevice");
				ad->ahiopen = !OpenDevice("ahi.device", AHI_NO_UNIT, (struct IORequest *)ad->ahireq, 0);
				if (ad->ahiopen)
				{
					AHIBase = (struct Library *)ad->ahireq->ahir_Std.io_Device;

					Amiga_Trace("SNDDMA_Init: AHI_AllocAudio");
					ad->audioctrl = AHI_AllocAudio(AHIA_AudioID, AHI_DEFAULT_ID,
					                               AHIA_MixFreq, speed,
					                               AHIA_Channels, 1,
					                               AHIA_Sounds, 1,
					                               TAG_END);

					if (ad->audioctrl)
					{
						Amiga_Trace("SNDDMA_Init: AHI_GetAudioAttrs");
						AHI_GetAudioAttrs(AHI_INVALID_ID, ad->audioctrl,
						                  AHIDB_BufferLen, sizeof(modename),
						                  AHIDB_Name, (ULONG)modename,
						                  AHIDB_MaxChannels, (ULONG)&channels,
						                  AHIDB_Bits, (ULONG)&bits,
						                  TAG_END);

						{
							ULONG mixfreq = 0;

							Amiga_Trace("SNDDMA_Init: MixFreq_Query");
							AHI_ControlAudio(ad->audioctrl,
							                 AHIC_MixFreq_Query, (ULONG)&mixfreq,
							                 TAG_END);

							/* The Paula driver has been seen to answer
							   with its 3.5 MHz clock here; only take a
							   mixing rate that makes sense. */
							if (mixfreq >= 8000 && mixfreq <= 96000)
								speed = mixfreq;
						}

						/* The streamed MP3s (music, the keep-as-MP3 speech)
						   decode only to 44100 Hz or exactly a half or a
						   quarter of it (towave.c); Paula's 14 bit modes mix
						   at 22030 Hz, and the music failed to start. Close
						   enough rates become the standard one: AHI resamples
						   to its mixing rate, and 0.1% is inaudible. */
						{
							static const ULONG stdrates[] = { 11025, 22050, 44100 };
							int i;

							for (i = 0; i < 3; i++)
							{
								if (speed != stdrates[i] && speed * 100 >= stdrates[i] * 99 && speed * 100 <= stdrates[i] * 101)
									speed = stdrates[i];
							}
						}

						/* AHIDB_Bits is the hardware mode's resolution
						   (8 for Paula DMA, 14 for Paula 14-bit, ...).
						   Always hand AHI 16-bit samples and let it
						   convert: the mixer writes 8-bit output
						   unsigned, but AHI's 8-bit sample types are
						   signed. */
						bits = 16;

						{
							/* Ring buffer length in sample frames. The mixer masks
							   positions with dma.samples - 1, so it must be a power
							   of two. It used to be 4096 * (speed / 11025) samples,
							   which was 93 ms at the 22030 Hz of AHI's Paula 14 bit
							   modes (integer division), far less than the half
							   second the mixer paints ahead: it painted over what
							   AHI was playing, and every frame longer than the
							   buffer ran it dry, so the sound crackled, the more
							   the lower the frame rate. Now about 0.75 s. */
							buffersize = speed > 32000 ? 32768 : 16384;

							if (channels > 2)
								channels = 2;

							dma.speed = speed;
							dma.samplebits = bits;
							dma.channels = channels;
							dma.samples = buffersize*channels;
							dma.submission_chunk = 1;

							ad->samplebuffer = AllocVec(buffersize*(bits/8)*channels, MEMF_ANY|MEMF_CLEAR);
							if (ad->samplebuffer)
							{
								dma.buffer = ad->samplebuffer;

								if (channels == 1)
								{
									if (bits == 8)
										sample.ahisi_Type = AHIST_M8S;
									else
										sample.ahisi_Type = AHIST_M16S;
								}
								else
								{
									if (bits == 8)
										sample.ahisi_Type = AHIST_S8S;
									else
										sample.ahisi_Type = AHIST_S16S;
								}

								sample.ahisi_Address = ad->samplebuffer;
								sample.ahisi_Length = buffersize;

								Amiga_Trace("SNDDMA_Init: AHI_LoadSound");
								r = AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample, ad->audioctrl);
								if (r == 0)
								{
									Amiga_Trace("SNDDMA_Init: AHIC_Play");
									r = AHI_ControlAudio(ad->audioctrl,
									                     AHIC_Play, TRUE,
									                     TAG_END);

									if (r == 0)
									{
										Amiga_Trace("SNDDMA_Init: AHI_Play");
										AHI_Play(ad->audioctrl,
										         AHIP_BeginChannel, 0,
										         AHIP_Freq, speed,
										         AHIP_Vol, 0x10000,
										         AHIP_Pan, 0x8000,
										         AHIP_Sound, 0,
										         AHIP_EndChannel, 0,
										         TAG_END);

										ad->aci.aeci.ahie_Effect = AHIET_CHANNELINFO;
										ad->aci.aeci.ahieci_Func = &ad->EffectHook;
										ad->aci.aeci.ahieci_Channels = 1;

										ad->EffectHook.h_Entry = (HOOKFUNC)EffectFunc;
										ad->EffectHook.h_Data = ad;

										Amiga_Trace("SNDDMA_Init: AHI_SetEffect");
										AHI_SetEffect(&ad->aci, ad->audioctrl);
										Amiga_Trace("SNDDMA_Init: done");

										Com_Printf("Using AHI mode \"%s\" for audio output\n", modename);
										Com_Printf("Channels: %d bits: %d frequency: %d\n", channels, bits, speed);

										return 1;
									}
								}
							}
							FreeVec(ad->samplebuffer);
						}
						AHI_FreeAudio(ad->audioctrl);
					}
					else
						Com_Printf("Failed to allocate AHI audio\n");

					CloseDevice((struct IORequest *)ad->ahireq);
				}
				DeleteIORequest((struct IORequest *)ad->ahireq);
			}
			DeleteMsgPort(ad->msgport);
		}
		FreeVec(ad);
	}

	return 0;
}

int SNDDMA_GetDMAPos(void)
{
	return ad->readpos*dma.channels;
}

void SNDDMA_Shutdown(void)
{
	struct Library *AHIBase;

	if (ad == 0)
		return;

	Amiga_Trace("SNDDMA_Shutdown");

	AHIBase = (struct Library *)ad->ahireq->ahir_Std.io_Device;

	Amiga_Trace("SNDDMA_Shutdown: cancel channel info effect");
	ad->aci.aeci.ahie_Effect = AHIET_CHANNELINFO|AHIET_CANCEL;
	AHI_SetEffect(&ad->aci.aeci, ad->audioctrl);
	Amiga_Trace("SNDDMA_Shutdown: stop playback");
	AHI_ControlAudio(ad->audioctrl,
	                 AHIC_Play, FALSE,
	                 TAG_END);

	Amiga_Trace("SNDDMA_Shutdown: AHI_FreeAudio");
	AHI_FreeAudio(ad->audioctrl);
	Amiga_Trace("SNDDMA_Shutdown: close ahi.device");
	FreeVec(ad->samplebuffer);
	CloseDevice((struct IORequest *)ad->ahireq);
	DeleteIORequest((struct IORequest *)ad->ahireq);
	DeleteMsgPort(ad->msgport);
	FreeVec(ad);

	ad = 0;
	Amiga_Trace("SNDDMA_Shutdown done");
}

void SNDDMA_Submit(void)
{
}

void SNDDMA_BeginPainting (void)
{
}
