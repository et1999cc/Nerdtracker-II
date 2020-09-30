#ifndef __NESESS_H_INCLUDED
#define __NESESS_H_INCLUDED

#define WAP     1

#include "typedefs.h"

#define VBLANK_SPEED_NTSC   60
#define VBLANK_SPEED_PAL    50

#define SNDDEV_NONE     0
#define SNDDEV_GUS      1
#define SNDDEV_SB       2
#define SNDDEV_PCSPK    3

//#define NUM_CHANNELS        5
#define NUM_CHANNELS        8
//#define NUM_A_CHANNELS      4
#define NUM_A_CHANNELS      7
#define SOUND_CONST_NTSC    111860.78
//#define SOUND_CONST_PAL     111860.78
#define SOUND_CONST_PAL     110840.47

#define SQRWAV1_CHN     0
#define SQRWAV2_CHN     1
#define TRIWAV_CHN      2
#define NOSWAV_CHN      3
#define DPCM_CHN        4

#define VRC6_SQRW1_CHN  5
#define VRC6_SQRW2_CHN  6
#define VRC6_SAW_CHN    7

#define SND_DEV_16BIT           0x01
#define SND_DEV_UNSIGNEDOUTPUT  0x02

int     SND_SoundSetup(void);
int     SND_SoundShutdown(void);


int     NesessInitialize(void);
int     NesessShutdown(void);

extern  void    (*CallInsideMixer)();
extern  int     VBlankSpeed;
extern  int     PlayBackRate;

extern	int		SoundConstIsPAL;

extern  int     EmulateVRC6;

extern  long    MixingRate;

void    Mix(void *Buffer, int Length, int Is16Bit, int StereoMode);
void    Write2SoundReg(int RegAddr, int Val);
int     ReadSoundReg(int RegAddr);

float   GetFrequency(int Chn);

extern  int     ChannelActive[NUM_CHANNELS];
extern  int     ChannelAmplitude[NUM_CHANNELS];

extern  void    *MixBuffer[2];

extern  void    *UpperPrgPage[4];

extern	int		NumFramesPlayed;


#endif

