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

#include "forte.h"

#include "gf1proto.h"
#include "extern.h"
#include "ultraerr.h"

#include "snd_gus.h"


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


#define VOICECONTROL_STOP               0x02
#define VOICECONTROL_IRQ_ENABLE         0x20
#define VOICECONTROL_LOOP_ENABLE        0x08

#define VOLUMECONTROL_ROLLOVER          0x04

#define GUS_DRAMLocationLo(x)   (long)((x<<9) & 0xFFFF)
#define GUS_DRAMLocationHi(x)   (long)((x>>7) & 0xFFFF)
#define GUS_GetRate(x)          (long)((((x<<9L)+(22050L))/44100L)<<1L)


int     GUS_BaseAddr=0x240,
        GUS_IRQ=7,
        GUS_DMA=3;


SND_DeviceTable *GUS;


char    IRQ_Table[16]={0, 0, 1, 3, 0, 2, 0, 4, 0, 0, 0, 5, 6, 0, 0, 7};
char    DMA_Table[8]={0, 1, 0, 2, 0, 3, 4, 5};

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
        }

void    GUS_WriteChnReg16(int Chn, int Reg, unsigned int Val)
        {
        GUS_Outp(GUS_PAGEREG, Chn);
        GUS_Outp(GUS_REGSELECT, Reg);
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
        GUS_Outp(GUS_REGSELECT, Reg+0x80);
        return(GUS_Inp(GUS_GLOBALDATA_HI));
        }

unsigned int GUS_ReadGloReg16(int Reg)
        {
        GUS_Outp(GUS_REGSELECT, Reg+0x80);
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


        /* (void)inp (_gf1_data.irq_status);
	outp(select,DMA_CONTROL);
	(void)inp (data_hi);
	outp(select,SAMPLE_CONTROL);
	(void)inp (data_hi);
	outp(select,GET_IRQV);
        (void)inp (data_hi); */


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

        GUS_Poke(0x100, 0x47);
        if(GUS_Peek(0x100)!=0x47) return(0);

        GUS_Poke(0x100, 47);
        if(GUS_Peek(0x100)!=47) return(0);

        return(1);
        }

void    GUS_ChnPlay(int Chn, long StartAddr, long EndAddr, long LoopAddr, long Rate, int Volume, int Panning, int Type)
        {
        GUS_WriteChnReg8(Chn, GUS_CHN_VOICECONTROL, VOICECONTROL_STOP);
        GUS_WriteChnReg8(Chn, GUS_CHN_VOICECONTROL, Type);

        GUS_WriteChnReg16(Chn, GUS_CHN_CURRENTADDR_LO, (StartAddr & 0xFFFF));
        GUS_WriteChnReg16(Chn, GUS_CHN_CURRENTADDR_HI, (StartAddr>>16));

        GUS_WriteChnReg16(Chn, GUS_CHN_ENDADDR_LO, (EndAddr & 0xFFFF));
        GUS_WriteChnReg16(Chn, GUS_CHN_ENDADDR_HI, (EndAddr>>16));

        GUS_WriteChnReg16(Chn, GUS_CHN_STARTADDR_LO, (LoopAddr & 0xFFFF));
        GUS_WriteChnReg16(Chn, GUS_CHN_STARTADDR_HI, (LoopAddr>>16));

        GUS_WriteChnReg16(Chn, GUS_CHN_VOLUME, Volume);
        GUS_WriteChnReg16(Chn, GUS_CHN_RATECONTROL, Rate);

        GUS_WriteChnReg16(Chn, GUS_CHN_PANNING, Panning);
        GUS_WriteChnReg8(Chn, GUS_CHN_VOICECONTROL, Type);
        }


// int     GUS_ReceiveDMA(long BufferAddr, long ElementLength, long Rate, int BufferPlaying, int Is16Bit)
int     GUS_CallBefore(int CurrBuffNum)
        {
        int     Is16Bit=(*GUS).Flags & 0x01;
        signed char     *Buffer8=(signed char *)(*GUS).BufferAddr;
        signed short    *Buffer16=(signed short *)(*GUS).BufferAddr;
        long            Buffer0=0, Buffer1=(*GUS).ElementLength<<Is16Bit;
        unsigned char *vidmem=(unsigned char *)0xB8000;

        if(CurrBuffNum==0) { // Rollover IRQ, buffer #1 playing automatically
//          if(Is16Bit) {
//            GUS_Poke(30, Buffer8[Buffer1+(ElementLength<<Is16Bit)-2]);
//            GUS_Poke(31, Buffer8[Buffer#1 DataLength*2-1]);}
//          else
//            GUS_Poke(31, Buffer8[Buffer#1 DataLength-1]);


          // Setup next IRQ
//          GUS_WriteChnReg8(0, GUS_CHN_VOLUMECONTROL, GUS_ReadChnReg8(0, GUS_CHN_VOLUMECONTROL) & (~VOLUMECONTROL_ROLLOVER));
//          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, ((32+Buffer1+(*GUS).ElementLength<<Is16Bit)>>16));
//          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LOl ((32+Buffer1+(*GUS).ElementLength<<Is16Bit) & 0xFFFF));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi((32+Buffer1+(*GUS).ElementLength<<Is16Bit)));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo((32+Buffer1+(*GUS).ElementLength<<Is16Bit)));

          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, (GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL) | VOICECONTROL_IRQ_ENABLE | VOICECONTROL_LOOP_ENABLE) & (~0x3));
          

          // Mix Buffer #0 (to be played in next phase)
          return(0x04);    // Tell mixer to mix & DMA-send buffer #0

          // Setup DMA transfer to Buffer #0
          // DMA_SetDMAC(GUS_DMA, BufferAddr+Buffer0, ElementLength<<Is16Bit, DMA_MODE_SIGNAL, DMA_OPERATION_READ)
          }
        else if(CurrBuffNum==1) { // Wavetable IRQ, buffer #0 playing automatically

                vidmem[160*24+80+8+1]=0x56;
          //if(Is16Bit) {
          //  GUS_Poke(30, Buffer8[Buffer#1 DataLength*2-2]);
          //  GUS_Poke(31, Buffer8[Buffer#1 DataLength*2-1]);}
          //  else
          //  GUS_Poke(31, Buffer8[Buffer#1 DataLength-1]);

          // Setup next IRQ
//          GUS_WriteChnReg8(0, GUS_CHN_VOLUMECONTROL, (VOLUMECONTROL_ROLLOVER));
//          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, ((32+Buffer0+(*GUS).ElementLength<<Is16Bit)>>16));
//          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, ((32+Buffer0+(*GUS).ElementLength<<Is16Bit)));

          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi((32+Buffer0+(*GUS).ElementLength<<Is16Bit)));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo((32+Buffer0+(*GUS).ElementLength<<Is16Bit)));
// THIS LINE SUCKS!(?)
          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, ((GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL) | (VOICECONTROL_IRQ_ENABLE)) & (~VOICECONTROL_LOOP_ENABLE)) & (~0x03));
//          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, (GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL) & (~VOICECONTROL_IRQ_ENABLE) & (~VOICECONTROL_LOOP_ENABLE)));

          // Mix Buffer #1 (to be played in next phase)
          return(0x04+0x01+0x02);    // Tell mixer to mix buffer #1
          // Setup DMA transfer to Buffer #1
          // DMA_SetDMAC(GUS_DMA, BufferAddr[Buffer#1], DataLength, DMA_MODE_SIGNAL, int DMA_OPERATION_READ);
          }


        // Don't forget to play the thing...
        // GUS_ChnPlay(Chn, StartAddr, EndAddr, LoopAddr);
        return(-1);
        }

int     GUS_CallAfter(int CurrBuffNum)
        {
        int     Is16Bit=(*GUS).Flags & 0x01;
        if(CurrBuffNum==0) {
          GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, (32>>(4+((GUS_DMA & 0x04)>>2))));
          // Enable DMA (GO!)
          GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x88+0x01);
          }
        else if(CurrBuffNum==1) {
          GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, ((32+(*GUS).ElementLength)>>(4+((GUS_DMA & 0x04)>>2))));
          // Enable DMA (GO!)
          GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x88+0x01);
          }
        return(0);
        }

void    GUS_StartOutput(void)
        {
        int     Is16Bit=(*GUS).Flags & 0x01;

        void    far *dmabuff=MK_FP((MixBuffer[0] & 0xFFFF0000)>>4, (MixBuffer[0] & 0xFFFF));
        UltraDownload(dmabuff,control,32,1024,TRUE);
        // Clear GUS DRAM
        /* {
        long    i;
        for(i=0; i<1048576; i++) GUS_Poke(i, 0);
        }*/

          // Check if GUS_WriteGloRegxx really does work...
          // GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, (32>>(4+((GUS_DMA & 0x04)>>2))));
          //^^GUS_WriteGloReg16(GUS_GLO_DMA_STARTADDR, (32>>4));
          // Enable DMA (GO!)
          //^^   GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x88+0x01);
          // GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x18+0x01);
          // GUS_WriteGloReg8(GUS_GLO_DRAM_DMACONTROL, 0x18+0x01+0x20);
    // DMA IRQ seems to work... DMA should work, but somehow doesn't. :(
    // Does not transfer at all. (but does give an IRQ at end of t. if desired)

        GUS_WriteChnReg16(0, GUS_CHN_VOLUME, 0xF1E0);
        GUS_WriteChnReg16(0, GUS_CHN_RATECONTROL, GUS_GetRate((*GUS).MixingRate));
        

//          GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_HI, GUS_DRAMLocationLo((32>>16)));
//          GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_LO, GUS_DRAMLocationLo((32 & 0xFFFF)));

          GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_HI, GUS_DRAMLocationHi(32));
          GUS_WriteChnReg16(0, GUS_CHN_CURRENTADDR_LO, GUS_DRAMLocationLo(32));

//          GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_HI, (32>>16));
//          GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_LO, (32 & 0xFFFF));

          GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_HI, GUS_DRAMLocationHi(32));
          GUS_WriteChnReg16(0, GUS_CHN_STARTADDR_LO, GUS_DRAMLocationLo(32));

          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi(32+(*GUS).ElementLength<<Is16Bit));
          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo(32+(*GUS).ElementLength<<Is16Bit));
//          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_HI, GUS_DRAMLocationHi(256000));
//          GUS_WriteChnReg16(0, GUS_CHN_ENDADDR_LO, GUS_DRAMLocationLo(256000));


          // Setup next IRQ
//          GUS_WriteChnReg8(0, GUS_CHN_VOLUMECONTROL, (VOLUMECONTROL_ROLLOVER));
//          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL,   (GUS_ReadChnReg8(0, GUS_CHN_VOICECONTROL) & (~VOICECONTROL_IRQ_ENABLE) & (~VOICECONTROL_LOOP_ENABLE)));
//(???) GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, VOICECONTROL_LOOP_ENABLE);
          // REMOVE!!!
//          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, ((VOICECONTROL_IRQ_ENABLE) | (VOICECONTROL_LOOP_ENABLE)));
// THIS LINE SUCKS TOO!(?)
          GUS_WriteChnReg8(0, GUS_CHN_VOICECONTROL, (0 | (VOICECONTROL_IRQ_ENABLE)));

        //void    GUS_ChnPlay(int Chn, long StartAddr, long EndAddr, long LoopAddr, long Rate, int Volume, int Panning, int Type)
        }

int     GUS_Initialize(void)
        {
        static  SecondTime=0;
        int     mix_image=0x09, irq_control=IRQ_Table[GUS_IRQ],
                dma_control=DMA_Table[GUS_DMA], Bajs;

        if(!GUS_Reset(14)) return(0);

        if(SecondTime) {
        Bajs=inp(0x21);
        Bajs|=(1<<GUS_IRQ);
        return(1);}

        //GUS_Outp(GUS_MIXCONTROL, (GUS_Inp(GUS_MIXCONTROL) | 0x40));
        //GUS_Outp(GUS_IRQANDDMACONTROL, IRQ_Table[GUS_IRQ]);
        //GUS_Outp(GUS_MIXCONTROL, (GUS_Inp(GUS_MIXCONTROL) & 0xBF));
        //GUS_Outp(GUS_IRQANDDMACONTROL, DMA_Table[GUS_DMA]);


/* Set up for Digital ASIC */
        outp(GUS_BaseAddr+0x0f,0x5);
        outp(GUS_BaseAddr+GUS_MIXCONTROL,mix_image);
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL,0x0);
        outp(GUS_BaseAddr+0x0f,0x0);

/* First do DMA control register */
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, dma_control|0x80|0x40);

/* IRQ CONTROL REG */
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image|0x40);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, irq_control);
	
/* First do DMA control register */
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, dma_control|0x40);

/* IRQ CONTROL REG */
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image|0x40);             
        outp(GUS_BaseAddr+GUS_IRQANDDMACONTROL, irq_control);
	
/* IRQ CONTROL, ENABLE IRQ */
/* just to Lock out writes to irq\dma register ... */
        outp(GUS_BaseAddr+GUS_PAGEREG, 0 );

/* enable output & irq, disable line & mic input */
	mix_image |= 0x09;
        outp(GUS_BaseAddr+GUS_MIXCONTROL, mix_image );             

/* just to Lock out writes to irq\dma register ... */
        outp(GUS_BaseAddr+GUS_PAGEREG, 0x0 );

	/* put image back .... */
        //_gf1_data.mix_image = mix_image;

        Bajs=inp(0x21);
        Bajs&=~(1<<GUS_IRQ);
        outp(0x21, Bajs);
        // outp(0x20, 0x20);

        SecondTime++;

        // Clear GUS DRAM
        {
        long    i;
        for(i=0; i<1048576; i++) GUS_Poke(i, 0);
        }
        return(1);
        }

char    Hexx[17]="0123456789ABCDEF";
void    GUS_AcknowledgeIRQ(void)
        {
        long    RegVar=0, Addr;
        unsigned char *vidmem=(unsigned char *)0xB8000;
        while(((GUS_ReadChnReg8(0, GUS_CHN_IRQSTATUS)) & 0xC0)!=0xC0) {
                vidmem[160*24+80]++;
                vidmem[160*24+80+1]=0x41;
                }

        // Print current DRAM location
        //RegVar=(GUS_ReadChnReg16(0, GUS_CHN_CURRENTADDR_HI)<<16);
        //RegVar+=GUS_ReadChnReg16(0, GUS_CHN_CURRENTADDR_LO);
        Addr=(RegVar>>9);
        vidmem[160*24+(75)*2]=Hexx[Addr & 0xF];
        vidmem[160*24+(75-1)*2]=Hexx[(Addr>>4) & 0xF];
        vidmem[160*24+(75-2)*2]=Hexx[(Addr>>8) & 0xF];
        vidmem[160*24+(75-3)*2]=Hexx[(Addr>>12) & 0xF];
        vidmem[160*24+(75-4)*2]=Hexx[(Addr>>16) & 0xF];
        vidmem[160*24+(75-5)*2]=Hexx[(Addr>>20) & 0xF];
        vidmem[160*24+(75-6)*2]=Hexx[(Addr>>24) & 0xF];
        vidmem[160*24+(75-7)*2]=Hexx[(Addr>>28) & 0xF];
        }

int     GUS_AutoDetect(SND_DeviceTable *DevTab)
        {
        int     i, j;
        char    *EnvVar;

{
char filename[80];
char temp_char[10];
unsigned char control;
unsigned char rec_control;
void *data_ptr;
void *vdata_ptr;
int fd;
int dram_len;
int temp;
int done;
int select;
int voice_num = 0;		/* examples use voice #0 */
unsigned int val;
unsigned int temp_val;
unsigned long dram_loc;
unsigned long begin,start_loop,end_loop;
unsigned long frequency;
unsigned long pos;
ULTRA_CFG config;
/* Get the ULTRASND environment string parameters */
UltraGetCfg(&config);

if (UltraProbe(config.base_port) == NO_ULTRA)
	{
	printf("No card found\n");
	exit(-1);
	}

if (UltraOpen(&config,14) == NO_ULTRA)
	{
	printf("No card found\n");
	exit(-1);
	}
}


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
        (*DevTab).Flags=0;

        (*DevTab).Initialize=GUS_Initialize;
        (*DevTab).AcknowledgeIRQ=GUS_AcknowledgeIRQ;
        (*DevTab).CallBefore=GUS_CallBefore;
        (*DevTab).CallAfter=GUS_CallAfter;
        (*DevTab).StartOutput=GUS_StartOutput;

        GUS=DevTab;
        return(1);
        }

