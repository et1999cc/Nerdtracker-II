#include <i86.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h> 
#include <dos.h>
#include <malloc.h>
#include <string.h>

/* #define __cplusplus
#include "osproto.h"
#include "forte.h"
#include "dma.h"
#include "gf1proto.h"
#include "extern.h"
#include "ultraerr.h"*/


#include "snd_gus.h"
#include "mydma.h"

#define GUS_PAGEREG             0x102
#define GUS_REGSELECT           0x103
#define GUS_GLOBALDATA_LO       0x104
#define GUS_GLOBALDATA_HI       0x105
#define GUS_IRQSTATUS           0x006
#define GUS_TIMERCONTROL        0x008
#define GUS_TIMERDATA           0x009
#define GUS_DRAM_IO             0x107
#define GUS_MIXCONTROL          0x000

#define GUS_IRQANDDMACONTROL    0x00B

#define GUS_GLO_DRAM_DMACONTROL 0x41
#define GUS_GLO_DMA_STARTADDR   0x42
#define GUS_GLO_DRAM_IOADDR_LO  0x43
#define GUS_GLO_DRAM_IOADDR_HI  0x44
#define GUS_GLO_TIMERCONTROL    0x45
#define GUS_GLO_TIMERCOUNT1     0x46
#define GUS_GLO_TIMERCOUNT2     0x47
#define GUS_GLO_SAMPLINGRATE    0x48
#define GUS_GLO_SAMPLINGCONTROL 0x49
#define GUS_GLO_JOYSTICKTRIMDAC 0x4B
#define GUS_GLO_RESET           0x4C

#define GUS_CHN_VOICECONTROL    0x00
#define GUS_CHN_RATECONTROL     0x01
#define GUS_CHN_STARTADDR_HI    0x02
#define GUS_CHN_STARTADDR_LO    0x03
#define GUS_CHN_ENDADDR_HI      0x04
#define GUS_CHN_ENDADDR_LO      0x05
#define GUS_CHN_VOLUMERAMP_RATE 0x06
#define GUS_CHN_VOLUMERAMP_START 0x07
#define GUS_CHN_VOLUMERAMP_END  0x08
#define GUS_CHN_VOLUME          0x09
#define GUS_CHN_CURRENTADDR_HI  0x0A
#define GUS_CHN_CURRENTADDR_LO  0x0B
#define GUS_CHN_PANNING         0x0C
#define GUS_CHN_VOLUMECONTROL   0x0D
#define GUS_CHN_MAX_CHANNELS    0x0E
#define GUS_CHN_IRQSTATUS       0x0F


#define DMACONTROL_16BIT                0x40

#define VOICECONTROL_STOP               0x02
#define VOICECONTROL_16BIT              0x04
#define VOICECONTROL_IRQ_ENABLE         0x20
#define VOICECONTROL_LOOP_ENABLE        0x08

#define VOLUMECONTROL_ROLLOVER          0x04

#define GUS_DRAMLocationLo(x)   (long)((x<<9) & 0xFFFF)
#define GUS_DRAMLocationHi(x)   (long)((x>>7) & 0xFFFF)
#define GUS_GetRate(x)          (long)((((x<<9L)+(22050L))/44100L)<<1L)


int     GUS_BaseAddr=0x240,
        GUS_IRQ=7,
        GUS_DMA=3;

int     SpeedOfDMA=0x00;

SND_DeviceTable *GUS;

char    IRQ_Table[16]={0, 0, 1, 3, 0, 2, 0, 4, 0, 0, 0, 5, 6, 0, 0, 7};
char    DMA_Table[8]={0, 1, 0, 2, 0, 3, 4, 5};

int     Voice=0,
        VoiceNeedsToBeServed=0;


void            GUS_Outp(int Reg, int Val);
void            GUS_Outpw(int Reg, unsigned int Val);
int             GUS_Inp(int Reg);
unsigned int    GUS_Inpw(int Reg);
void            GUS_Delay(void);
void            GUS_WriteChnReg8(int Chn, int Reg, int Val);
void            GUS_WriteChnReg16(int Chn, int Reg, unsigned int Val);
void            GUS_WriteGloReg8(int Reg, int Val);
void            GUS_WriteGloReg16(int Reg, unsigned int Val);
int             GUS_ReadChnReg8(int Chn, int Reg);
unsigned int    GUS_ReadChnReg16(int Chn, int Reg);
int             GUS_ReadGloReg8(int Reg);
unsigned int    GUS_ReadGloReg16(int Reg);
void            GUS_Poke(long Addr, int Val);
int             GUS_Peek(long Addr);
int             GUS_AutoDetect(SND_DeviceTable *DevTab);
void            GUS_DMAReceive(int Addr);

void    GUS_Outp(int Reg, int Val)
        {
        outp(GUS_BaseAddr+Reg, Val);
        }

void    GUS_Outpw(int Reg, unsigned int Val)
        {
        outpw(GUS_BaseAddr+Reg, Val);
        }

int     GUS_Inp(int Reg)
        {
        return(inp(GUS_BaseAddr+Reg));
        }

unsigned int     GUS_Inpw(int Reg)
        {
        return(inpw(GUS_BaseAddr+Reg));
        }

void    GUS_Delay(void)
        {
        GUS_Inp(GUS_MIXCONTROL);
        GUS_Inp(GUS_MIXCONTROL);
        GUS_Inp(GUS_MIXCONTROL);
        GUS_Inp(GUS_MIXCONTROL);
        GUS_Inp(GUS_MIXCONTROL);
        GUS_Inp(GUS_MIXCONTROL);
        GUS_Inp(GUS_MIXCONTROL);
        }

void    GUS_WriteChnReg8(int Chn, int Reg, int Val)
        {
        GUS_Outp(GUS_PAGEREG, Chn);
        GUS_Outp(GUS_REGSELECT, Reg);
        GUS_Outp(GUS_GLOBALDATA_HI, Val);
        GUS_Delay();
        GUS_Outp(GUS_GLOBALDATA_HI, Val);
        }

void    GUS_WriteChnReg16(int Chn, int Reg, unsigned int Val)
        {
        GUS_Outp(GUS_PAGEREG, Chn);
        GUS_Outp(GUS_REGSELECT, Reg);
        GUS_Outpw(GUS_GLOBALDATA_LO, Val);
        GUS_Delay();
        GUS_Outpw(GUS_GLOBALDATA_LO, Val);
        }

void    GUS_WriteGloReg8(int Reg, int Val)
        {
        GUS_Outp(GUS_REGSELECT, Reg);
        GUS_Outp(GUS_GLOBALDATA_HI, Val);
        GUS_Delay();
        GUS_Outp(GUS_GLOBALDATA_HI, Val);
        }

void    GUS_WriteGloReg16(int Reg, unsigned int Val)
        {
        GUS_Outp(GUS_REGSELECT, Reg);
        GUS_Outpw(GUS_GLOBALDATA_LO, Val);
        GUS_Delay();
        GUS_Outpw(GUS_GLOBALDATA_LO, Val);
        }

int     GUS_ReadChnReg8(int Chn, int Reg)
        {
        GUS_Outp(GUS_PAGEREG, Chn);
        GUS_Outp(GUS_REGSELECT, Reg+0x80);
        return(GUS_Inp(GUS_GLOBALDATA_HI));
        }

unsigned int GUS_ReadChnReg16(int Chn, int Reg)
        {
        GUS_Outp(GUS_PAGEREG, Chn);
        GUS_Outp(GUS_REGSELECT, Reg+0x80);
        return(GUS_Inpw(GUS_GLOBALDATA_LO));
        }

int     GUS_ReadGloReg8(int Reg)
        {
        GUS_Outp(GUS_REGSELECT, Reg);
        return(GUS_Inp(GUS_GLOBALDATA_HI));
        }

unsigned int GUS_ReadGloReg16(int Reg)
        {
        GUS_Outp(GUS_REGSELECT, Reg);
        return(GUS_Inpw(GUS_GLOBALDATA_LO));
        }

void    GUS_Poke(long Addr, int Val)
        {
        GUS_WriteGloReg16(GUS_GLO_DRAM_IOADDR_LO, (Addr & 0xFFFF));
        GUS_WriteGloReg8(GUS_GLO_DRAM_IOADDR_HI, (Addr>>16));
        GUS_Outp(GUS_DRAM_IO, Val);
        }

int     GUS_Peek(long Addr)
        {
        GUS_WriteGloReg16(GUS_GLO_DRAM_IOADDR_LO, (Addr & 0xFFFF));
        GUS_WriteGloReg8(GUS_GLO_DRAM_IOADDR_HI, (Addr>>16));
        return(GUS_Inp(GUS_DRAM_IO));
        }

int     GUS_Reset(int MaxChannels)
        {
        int     i, j;
        GUS_Outp(GUS_GLO_RESET, 0);
        GUS_Delay();
        GUS_Outp(GUS_GLO_RESET, 1);
        GUS_Delay();
        GUS_Outp(GUS_GLO_DRAM_DMACONTROL, 0);
        GUS_Outp(GUS_GLO_TIMERCONTROL, 0);
        GUS_Outp(GUS_GLO_SAMPLINGCONTROL, 0);

        if(MaxChannels<14) MaxChannels=14;
        if(MaxChannels>32) MaxChannels=32;

        GUS_Outp(GUS_CHN_MAX_CHANNELS, (MaxChannels-1));


         //GUS_Inp(GUS_CHN_IRQSTATUS);
        GUS_Inp(GUS_GLO_DRAM_DMACONTROL);
        GUS_Inp(GUS_GLO_SAMPLINGCONTROL);
        GUS_Inp(GUS_CHN_IRQSTATUS);

        for(i=0; i<MaxChannels; i++) {
          GUS_WriteChnReg8(i, GUS_CHN_VOICECONTROL, 3);
          GUS_WriteChnReg8(i, GUS_CHN_VOLUMECONTROL, 3);
          }

        //GUS_Inp(GUS_CHN_IRQSTATUS);
        GUS_Inp(GUS_GLO_DRAM_DMACONTROL);
        GUS_Inp(GUS_GLO_SAMPLINGCONTROL);
        GUS_Inp(GUS_CHN_IRQSTATUS);

        GUS_Outp(GUS_GLO_RESET, 7);

         GUS_Outp(GUS_MIXCONTROL, 0x09);

        //GUS_Poke(0x100, 0x47);
        //if(GUS_Peek(0x100)!=0x47) return(0);

        //GUS_Poke(0x100, 48);
        //if(GUS_Peek(0x100)!=48) return(0);

        return(1);
        }

void    GUS_DMAReceive(int Addr)
        {
        int     Is16Bit=(*GUS).Flags & 0x01;

        GUS_ReadGloReg8(GUS_GLO_DRAM_DMACONTROL);       
        Addr>>=4;
        if(GUS_DMA & 4)
          Addr>>=1;
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x00);
        GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, Addr);
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, SpeedOfDMA|(GUS_DMA&4)|0x80|(Is16Bit?0x40:0x00)|0x1);

        //dmaStart(gusDMA, (void *)buf, len, 0x48);
        }

int     GUS_CallBefore(int CurrBuffNum)
        {
        int     Is16Bit=(*GUS).Flags & 0x01;
        signed char     *Buffer8=(signed char *)(*GUS).BufferAddr;
        signed short    *Buffer16=(signed short *)(*GUS).BufferAddr;
        long            Buffer0=0, Buffer1=(*GUS).ElementLength<<Is16Bit;
        unsigned char *vidmem=(unsigned char *)0xB8000;

/*        if(CurrBuffNum==0) { // Rollover IRQ, buffer #1 playing automatically
//          if(Is16Bit) {
//            GUS_Poke(30, Buffer8[Buffer1+(ElementLength<<Is16Bit)-2]);
//            GUS_Poke(31, Buffer8[Buffer#1 DataLength*2-1]);}
//          else
//            GUS_Poke(31, Buffer8[Buffer#1 DataLength-1]);


          // Setup next IRQ
          // Rollover has apparently not worked in the past...
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi(((32+Buffer1+((*GUS).ElementLength<<Is16Bit))>>Is16Bit)-1));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo(((32+Buffer1+((*GUS).ElementLength<<Is16Bit))>>Is16Bit)-1));
          GUS_WriteChnReg8(0, GUS_CHN_VOLUMECONTROL, GUS_ReadChnReg8(0, GUS_CHN_VOLUMECONTROL) & (~VOLUMECONTROL_ROLLOVER));

          // GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, (GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL) | VOICECONTROL_IRQ_ENABLE | VOICECONTROL_LOOP_ENABLE));
          {
          int   VCReg=GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL);
          if(VCReg & 0x01) {
            vidmem[10*160+78*2]++;
            vidmem[10*160+78*2+1]++;
            }
          VCReg=((VCReg & 0xFC) | VOICECONTROL_IRQ_ENABLE | VOICECONTROL_LOOP_ENABLE);
          if(Is16Bit) VCReg|=VOICECONTROL_16BIT;
          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, VCReg);
          }

        GUS_ReadGloReg8(GUS_GLO_DRAM_DMACONTROL);
        
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x00);
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x00);


          // Mix Buffer #0 (to be played in next phase)
          return(0x04);    // Tell mixer to mix & DMA-send buffer #0
          }
        else if(CurrBuffNum==1) { // Wavetable IRQ, buffer #0 playing automatically

                vidmem[160*24+80+8+1]=0x56;
          //if(Is16Bit) {
          //  GUS_Poke(30, Buffer8[Buffer#1 DataLength*2-2]);
          //  GUS_Poke(31, Buffer8[Buffer#1 DataLength*2-1]);}
          //  else
          //  GUS_Poke(31, Buffer8[Buffer#1 DataLength-1]);

          // Setup next IRQ
          // Rollover has apparently not worked in the past...
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi((32+Buffer0+((*GUS).ElementLength<<Is16Bit))>>Is16Bit));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo((32+Buffer0+((*GUS).ElementLength<<Is16Bit))>>Is16Bit));

          GUS_WriteChnReg8(0, GUS_CHN_VOLUMECONTROL, (VOLUMECONTROL_ROLLOVER));



//          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, (GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL) & (~VOICECONTROL_IRQ_ENABLE) & (~VOICECONTROL_LOOP_ENABLE)));
          {
          int   VCReg=GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL);
          if(VCReg & 0x01) {
            vidmem[10*160+78*2]++;
            vidmem[10*160+78*2+1]++;
            }
          VCReg=(((VCReg & 0xFC) | VOICECONTROL_IRQ_ENABLE) & (~VOICECONTROL_LOOP_ENABLE));
          if(Is16Bit) VCReg|=VOICECONTROL_16BIT;
          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, VCReg);
          }

        GUS_ReadGloReg8(GUS_GLO_DRAM_DMACONTROL);
        
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x00);
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x00);

          // Mix Buffer #1 (to be played in next phase)
          return(0x04+0x01+0x02);    // Tell mixer to mix buffer #1
          }*/

        if((Voice==1) && (VoiceNeedsToBeServed)) {
          //GUS_DMAReceive(0);
          return(0x04);    // Tell mixer to mix & DMA-send buffer #0
          }
        if((Voice==0) && (VoiceNeedsToBeServed)) {
          //GUS_DMAReceive(((*GUS).ElementLength<<Is16Bit));
          return(0x04+0x01+0x02);
          }
        return(-1);
        }

void    GUS_CallAfter(int CurrBuffNum)
        {
        int     Is16Bit=(*GUS).Flags & 0x01;
        
        /*if(CurrBuffNum==0) {
          GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, (32>>(4+((GUS_DMA & 0x04)>>2))));
          }
        else if(CurrBuffNum==1) {
          GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, ((32+((*GUS).ElementLength<<Is16Bit))>>(4+((GUS_DMA & 0x04)>>2))));
          }
          // Enable DMA (GO!)
        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, SpeedOfDMA+0x80+0x01+(GUS_DMA & 0x04)+(DMACONTROL_16BIT & (Is16Bit<<6)));
//        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, SpeedOfDMA+0x01+(GUS_DMA & 0x04)+(DMACONTROL_16BIT & (Is16Bit<<6)));*/

        if((Voice==1) && (VoiceNeedsToBeServed)) {
          GUS_DMAReceive(0);
          //return(0x04);    // Tell mixer to mix & DMA-send buffer #0
          }
        if((Voice==0) && (VoiceNeedsToBeServed)) {
          GUS_DMAReceive(((*GUS).ElementLength<<Is16Bit));
          //return(0x04+0x01+0x02);
          }

        }

void    GUS_StartOutput(void)
        {
        int     i, j;
        int     Is16Bit=(*GUS).Flags & 0x01;
        int     buflen2=((*GUS).ElementLength<<Is16Bit);

        void    far *dmabuff=(void far *)(unsigned long)( ((unsigned int)(((unsigned int)MixBuffer[0] & 0xFFFF0000)>>4)<<16)+((unsigned int)MixBuffer[0] & 0xFFFF));
        printf("Norm: %#08x, Far: %#08x\n", (long)MixBuffer[0], (long)dmabuff);

          GUS_ReadGloReg8(GUS_GLO_DRAM_DMACONTROL);

          GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, (32>>(4+((GUS_DMA & 0x04)>>2))));
          GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, SpeedOfDMA+0x80+0x01+(GUS_DMA & 0x04)+(DMACONTROL_16BIT & (Is16Bit<<6)));


//        GUS_WriteChnReg16(0, GUS_CHN_VOLUME, 0xE5D0);
//        GUS_WriteChnReg16(0, GUS_CHN_VOLUME, 0xF4DE);

// Set up channel #0 and #1 to do IRQs
// Channel #0
GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_HI, GUS_DRAMLocationHi(0));
GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_LO, GUS_DRAMLocationLo(0));
GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_HI, GUS_DRAMLocationHi(0));
GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_LO, GUS_DRAMLocationLo(0));
GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi((buflen2<<1)>>Is16Bit));
GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo((buflen2<<1)>>Is16Bit));
GUS_WriteChnReg16(0, GUS_CHN_RATECONTROL, GUS_GetRate((*GUS).MixingRate));
GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, 0x28);

// Channel #1
GUS_WriteChnReg16(1, GUS_CHN_STARTADDR_HI, GUS_DRAMLocationHi(0));
GUS_WriteChnReg16(1, GUS_CHN_STARTADDR_LO, GUS_DRAMLocationLo(0));
GUS_WriteChnReg16(1, GUS_CHN_CURRENTADDR_HI, GUS_DRAMLocationHi(buflen2>>Is16Bit));
GUS_WriteChnReg16(1, GUS_CHN_CURRENTADDR_LO, GUS_DRAMLocationLo(buflen2>>Is16Bit));
GUS_WriteChnReg16(1, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi((buflen2<<1)>>Is16Bit));
GUS_WriteChnReg16(1, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo((buflen2<<1)>>Is16Bit));
GUS_WriteChnReg16(1, GUS_CHN_RATECONTROL, GUS_GetRate((*GUS).MixingRate));
GUS_WriteChnReg8(1, GUS_CHN_VOICECONTROL, 0x28);

// Then set up channel #2 to play the thing
GUS_WriteChnReg16(2, GUS_CHN_STARTADDR_HI, GUS_DRAMLocationHi(0));
GUS_WriteChnReg16(2, GUS_CHN_STARTADDR_LO, GUS_DRAMLocationLo(0));
GUS_WriteChnReg16(2, GUS_CHN_CURRENTADDR_HI, GUS_DRAMLocationHi(0));
GUS_WriteChnReg16(2, GUS_CHN_CURRENTADDR_LO, GUS_DRAMLocationLo(0));
GUS_WriteChnReg16(2, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi(buflen2>>(1-Is16Bit)));
GUS_WriteChnReg16(2, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo(buflen2>>(1-Is16Bit)));
GUS_WriteChnReg16(2, GUS_CHN_RATECONTROL, 1024);
//GUS_GetRate((*GUS).MixingRate));
GUS_WriteChnReg16(2, GUS_CHN_VOLUME, 0xFA9E);
GUS_WriteChnReg8(2, GUS_CHN_VOICECONTROL, (Is16Bit?0x0C:0x08));

        /*GUS_WriteChnReg16(0, GUS_CHN_VOLUME, 0xFA9E);
        GUS_WriteChnReg16(0, GUS_CHN_RATECONTROL, GUS_GetRate((*GUS).MixingRate));
        

          GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_HI, GUS_DRAMLocationHi(32>>Is16Bit));
          GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_LO, GUS_DRAMLocationLo(32>>Is16Bit));

          GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_HI, GUS_DRAMLocationHi((32>>Is16Bit)));
          GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_LO, GUS_DRAMLocationLo((32>>Is16Bit)));

          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi(((32+((*GUS).ElementLength<<Is16Bit))>>Is16Bit)-1));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo(((32+((*GUS).ElementLength<<Is16Bit))>>Is16Bit)-1));

          // Setup first IRQ
          GUS_WriteChnReg8(0, GUS_CHN_VOLUMECONTROL, (VOLUMECONTROL_ROLLOVER));
          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, (VOICECONTROL_IRQ_ENABLE | (VOICECONTROL_16BIT & (Is16Bit<<2))));*/

        /*for(i=0; i<32; i++) {
          GUS_Poke(i, 0xFF);
          GUS_Poke(512+32+i, 0x01);
          }*/
          
        }

int     GUS_Initialize(void)
        {
/*        static  SecondTime=0;
        int     mix_image=0x09, irq_control=IRQ_Table[GUS_IRQ],
                dma_control=DMA_Table[GUS_DMA], Bajs;

        if(!GUS_Reset(14)) {
            printf("failed to reset card!\n");
            return(0);}

        if(SecondTime) {
        if(GUS_IRQ & 0x08)  Bajs=inp(0xA1);
        else                Bajs=inp(0x21);
        Bajs|=(1<<(GUS_IRQ & 0x07));
        return(1);}

        //GUS_Outp(GUS_MIXCONTROL, (GUS_Inp(GUS_MIXCONTROL) | 0x40));
        //GUS_Outp(GUS_IRQANDDMACONTROL, IRQ_Table[GUS_IRQ]);
        //GUS_Outp(GUS_MIXCONTROL, (GUS_Inp(GUS_MIXCONTROL) & 0xBF));
        //GUS_Outp(GUS_IRQANDDMACONTROL, DMA_Table[GUS_DMA]);


// Set up for Digital ASIC
        outp(GUS_BaseAddr+0x0f,0x5);
        outp(GUS_BaseAddr+GUS_MIXCONTROL,mix_image);
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL,0x0);
        outp(GUS_BaseAddr+0x0f,0x0);

// First do DMA control register
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, dma_control|0x80|0x40);

// IRQ CONTROL REG
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image|0x40);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, irq_control);
	
// First do DMA control register
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, dma_control|0x40);

// IRQ CONTROL REG
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image|0x40);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, irq_control);
	
// IRQ CONTROL, ENABLE IRQ
// just to Lock out writes to irq\dma register ...
        outp(GUS_BaseAddr+GUS_PAGEREG, 0 );

// enable output & irq, disable line & mic input
	mix_image |= 0x09;
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image );             

// just to Lock out writes to irq\dma register ... 
        outp(GUS_BaseAddr+GUS_PAGEREG, 0x0 );

        // put image back .... 
        //_gf1_data.mix_image = mix_image;

        if(GUS_IRQ & 0x08)  Bajs=inp(0xA1);
        else                Bajs=inp(0x21);
        Bajs&=~(1<<(GUS_IRQ & 0x07));
        if(GUS_IRQ & 0x08) {
            outp(0xA1, Bajs);
            outp(0x21, 0x00);}
        else                outp(0x21, Bajs);
        outp(0x20, 0x20);
        outp(0xA0, 0x20);

        SecondTime++;

        // Clear GUS DRAM
        {
        long    i;
        for(i=0; i<16384; i++) GUS_Poke(i, 0x00);
        }

        GUS_ReadGloReg8(GUS_GLO_DRAM_DMACONTROL);

        GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x00);
        GUS_WriteGloReg8(GUS_GLO_TIMERCONTROL, 0x00);
        GUS_WriteGloReg8(GUS_GLO_SAMPLINGCONTROL, 0x00);*/
  int   voices=14, activevoices;
  int i;

  if (voices<14)
    voices=14;
  if (voices>32)
    voices=32;

  activevoices=voices;


  GUS_WriteGloReg8(0x4C, 0);
  for (i=0; i<10; i++)
    GUS_Delay();

//  outGUS(0x4C, 1);
  GUS_WriteGloReg8(0x4C, 1);
  for (i=0; i<10; i++)
    GUS_Delay();


  GUS_WriteGloReg8(0x41, 0);   // DMA Control
  GUS_WriteGloReg8(0x45, 0);   // Timer Control
  GUS_WriteGloReg8(0x49, 0);   // Sampling Control

  //outGUS(0xE, (voices-1)|0xC0);
  GUS_WriteGloReg8(0xE, ((voices-1)|0xC0));

  //inpGUS(0x6);
  GUS_Inp(0x6);
  //inGUS(0x41);
  GUS_ReadGloReg8(0x41);
  //inGUS(0x49);
  GUS_ReadGloReg8(0x49);
  //inGUS(0x8F);
  GUS_ReadGloReg8(0x8F);

  for (i=0; i<32; i++)
  {
    //selvoc(i);
    //setvol(0);  // vol=0
    //setmode(3);  // stop voice
    //setvmode(3);  // stop volume
    //setpoint8(0,0);

    GUS_WriteChnReg16(i, GUS_CHN_VOLUME, 0x00);
    GUS_WriteChnReg8(i, GUS_CHN_VOICECONTROL, 3);
    GUS_WriteChnReg8(i, GUS_CHN_VOLUMECONTROL, 3);
    GUS_WriteChnReg16(i, GUS_CHN_STARTADDR_HI, GUS_DRAMLocationHi(0));
    GUS_WriteChnReg16(i, GUS_CHN_STARTADDR_LO, GUS_DRAMLocationLo(0));

    GUS_Delay();
  }

  //inpGUS(0x6);
  GUS_Inp(0x6);
  //inGUS(0x41);
  GUS_ReadGloReg8(0x41);
  //inGUS(0x49);
  GUS_ReadGloReg8(0x49);
  //inGUS(0x8F);
  GUS_ReadGloReg8(0x8F);

  GUS_WriteGloReg8(0x4C, 7);

//  unsigned char l1="\x00\x00\x01\x03\x00\x02\x00\x04\x00\x00\x00\x05\x06\x00\x00\x07"[gusIRQ]|((gusIRQ==gusIRQ2)?0x40:"\x00\x00\x08\x18\x00\x10\x00\x20\x00\x00\x00\x28\x30\x00\x00\x38"[gusIRQ2]);
//  unsigned char l2="\x00\x01\x00\x02\x00\x03\x04\x05"[gusDMA]|((gusDMA==gusDMA2)?0x40:"\x00\x08\x00\x10\x00\x18\x20\x28"[gusDMA2]);

  GUS_Outp(0xF, 5);
  //outpGUSF(5);
  GUS_Outp(0x0, 0xB);
  //outpGUS0(0x0B);
  GUS_Outp(0xB, 0);
  //outpGUSB(0);
  GUS_Outp(0xF, 0);
  // outpGUSF(0);

  GUS_Outp(0x0, 0xB);
  //outpGUS0(0x0B);
  GUS_Outp(0xB, (0x80|DMA_Table[GUS_DMA]));
  //outpGUSB(l2|0x80);
  GUS_Outp(0x0, 0x4B);
  GUS_Outp(0xB, (IRQ_Table[GUS_IRQ]));
  //outpGUSB(l1);
  GUS_Outp(0x0, 0xB);
  //outpGUS0(0x0B);
  GUS_Outp(0xB, (DMA_Table[GUS_DMA]));
  //outpGUSB(l2);
  GUS_Outp(0x0, 0x4B);
  //outpGUS0(0x4B);
  GUS_Outp(0xB, (IRQ_Table[GUS_IRQ]));
//  outpGUSB(l1);

//  selvoc(0);
//  outpGUS0(0x08);
  GUS_Outp(0x0, 0x08);
//  selvoc(0);

        return(1);
        }


char    Hexx[17]="0123456789ABCDEF";
void    GUS_AcknowledgeIRQ(void)
        {
        int     IRQStatus;
        long    RegVar=0, Addr;
        unsigned char *vidmem=(unsigned char *)0xB8000;

        while(1) {
                IRQStatus=GUS_ReadChnReg8(0, GUS_CHN_IRQSTATUS);
                Voice=(IRQStatus & 0x1F);
                if(!(IRQStatus & 0x80))
                  VoiceNeedsToBeServed=1;
                if((IRQStatus & 0xC0)==0xC0)
                  break;
                }
          
        }

int     GUS_AutoDetect(SND_DeviceTable *DevTab)
        {
        int     i, j;
        char    *EnvVar;
        if((EnvVar=getenv("ULTRASND"))==0) {
        printf("ULTRASND environment variable not found!");
          return(0);
          }
        i=0;
        GUS_BaseAddr=0x200 | ((int)(EnvVar[i+1]-'0')<<4);
        while(EnvVar[i]!=',') i++;
        GUS_DMA=((int)(EnvVar[i+1]-'0'));
        i++;
        while(EnvVar[i]!=',') i++;
        i++;
        while(EnvVar[i]!=',') i++;
        if(EnvVar[i+2]!=',')
                GUS_IRQ=((int)(EnvVar[i+1]-'0'))*10+((int)(EnvVar[i+2]-'0'));
        else
                GUS_IRQ=((int)(EnvVar[i+1]-'0'));
        (*DevTab).BaseAddr=GUS_BaseAddr;
        (*DevTab).IRQ=GUS_IRQ;
        (*DevTab).DMA=GUS_DMA;
        (*DevTab).MaxMixingRate=44100;
        (*DevTab).Flags=0x01;

        (*DevTab).Initialize=GUS_Initialize;
        (*DevTab).AcknowledgeIRQ=GUS_AcknowledgeIRQ;
        (*DevTab).CallBefore=GUS_CallBefore;
        (*DevTab).CallAfter=GUS_CallAfter;
        (*DevTab).StartOutput=GUS_StartOutput;
        GUS=DevTab;


//
// {
// char filename[80];
// char temp_char[10];
// unsigned char control;
// unsigned char rec_control;
// void *data_ptr;
// void *vdata_ptr;
// int fd;
// int dram_len;
// int temp;
// int done;
// int select;
// int voice_num = 0;      // examples use voice #0
// unsigned int val;
// unsigned int temp_val;
// unsigned long dram_loc;
// unsigned long begin,start_loop,end_loop;
// unsigned long frequency;
// unsigned long pos;
// ULTRA_CFG config;
// Get the ULTRASND environment string parameters
// UltraGetCfg(&config);
// 
// if (UltraProbe(config.base_port) == NO_ULTRA)
//         {
//         printf("No card found\n");
//     //exit(-1);
//         }
// 
// if (UltraOpen(&config,14) == NO_ULTRA)
//         {
//         printf("No card found\n");
//     // exit(-1);
//         }
// 
// }

        return(1);
        }

