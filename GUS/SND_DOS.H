#ifndef __NESESS_H_INCLUDED
#define __NESESS_H_INCLUDED

#include "typedefs.h"

#define SNDDEV_NONE     0
#define SNDDEV_GUS      1
#define SNDDEV_SB       2
#define SNDDEV_PCSPK    3

#define SND_DEV_16BIT           0x01
#define SND_DEV_UNSIGNEDOUTPUT  0x02
#define SND_DEV_AIDMA           0x10

typedef _Packed struct {
int     (*Initialize)(void);
void    (*AcknowledgeIRQ)(void);
void    (*ReceiveDMA)(long Addr, long Length, long Rate);
int     (*CallBefore)(int CurrentBuffNum);
void    (*CallAfter)(int CurrentBuffNum);
void    (*StartOutput)(void);
WORD    BaseAddr;
BYTE    IRQ;
BYTE    DMA;
WORD    MaxMixingRate;
BYTE    Flags;

long    BufferAddr;
long    ElementLength;
long    MixingRate;
} SND_DeviceTable;


int     SND_SoundSetup(void);
int     SND_SoundShutdown(void);

extern  void    *MixBuffer[2];
extern  long    PreferredMixingRate;
#endif
