#include <i86.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>

#include "typedefs.h"
#include "mydma.h"
#include "imsdma.h"

#include "snd_gus.h"
#include "snd_sb.h"

// #include "nesess.h"


void    SND_SetupHandler(void);
void    SND_RestoreHandler(void);

SND_DeviceTable SND;

#define SND_DEV_AIDMA           0x10

extern  long    MixingRate=21700;
extern  double  MixingRateR;
long    PreferredMixingRate=0;
void    *MixBuffer[2]={NULL,NULL};
long    MixBufferSelector;
//int     MixBufferSize=32768;
int     MixBufferSize=1024;
int     PlayFlag=0;

int     SoundDevice=0;

int     IRQ_BaseLo, IRQ_BaseHi, IRQ_Base;

long    OldIRQ_Selector,
        OldIRQ_Offset;

void    __interrupt (*OldIRQ)();
void    __interrupt IRQ_Handler();

char    hextab[17]="0123456789ABCDEF";

int     SND_SoundSetup(void)
        {
        int i;
        int Is16Bit;
        memset(&SND, 0, sizeof(SND));
        if(!GUS_AutoDetect(&SND)) {
          if(!SB_AutoDetect(&SND))
            return(0);
          }
        if(SND.Initialize==0) return(0);

        if((SND.IRQ>7) || (SND.DMA>3)) {
                printf("\nSorry! This proggy is too lame to use high IRQs (8 and above)\nor 16-bit DMA channels (4 and above). \nPlease change them to lower ones (not too much trouble if you've got GUS/Interwave)\n\n");
                if(SND.IRQ>7) return(0);
                }

        printf("Port: %Xh, IRQ: %d, DMA: %d\n", (int)SND.BaseAddr, (int)SND.IRQ, (int)SND.DMA);

        printf("Autodetect seems ok?\n");
        if(!SND.Initialize())
          return(0);
        Is16Bit=(SND.Flags & SND_DEV_16BIT);

        SND_SetupHandler();

        SND.BufferAddr=(long)MixBuffer[0];
        SND.ElementLength=(long)MixBufferSize;
        if(PreferredMixingRate){
          //SND.MixingRate=SND.MaxMixingRate;
          SND.MixingRate=PreferredMixingRate;
          if(SND.MixingRate>SND.MaxMixingRate)
             SND.MixingRate=SND.MaxMixingRate;
          }
        else SND.MixingRate=SND.MaxMixingRate;

        MixingRate=SND.MixingRate;
        //clean buffer
        {
        /* int i;
        signed char   *Bf1=(signed char *)MixBuffer[0],
                        *Bf2=(signed char *)MixBuffer[1];

        for(i=0; i<MixBufferSize; i++) {
            Bf1[i]=rand()%256;
            Bf2[i]=rand()%256;
            // Bf1[i]=0;
            // Bf2[i]=0;

            }*/
        memset(MixBuffer[0], 0, (MixBufferSize<<Is16Bit)*2);
        //memset(MixBuffer[0], -127, (MixBufferSize<<Is16Bit));
        //memset(MixBuffer[1], 127, (MixBufferSize<<Is16Bit));
        }

        if(SND.IRQ<8)
          outp(0x21, (inp(0x21) & (~(1<<SND.IRQ))));
        else
          outp(0xA1, (inp(0xA1) & (~(1<<(SND.IRQ-8)))));

        // Really use autoinit?...
        if(SND.Flags & SND_DEV_AIDMA)
          DMA_SetDMAC(SND.DMA, (long)MixBuffer[PlayFlag], (MixBufferSize<<Is16Bit)*2, DMA_MODE_DEMAND+DMA_AUTOINIT, DMA_OPERATION_READ);
        else
          DMA_SetDMAC(SND.DMA, (long)MixBuffer[PlayFlag], (MixBufferSize<<Is16Bit)*2, DMA_MODE_DEMAND, DMA_OPERATION_READ);
        SND.StartOutput();


        return(1);
        }

int     SND_SoundShutdown(void)
        {
        if(SND.Initialize==0) return(0);
        SND.Initialize();
        SND_RestoreHandler();
        return(1);
        }

void    SND_SetupHandler(void) {
        long    CurrAddr, PageTop;
        union REGS r;
        struct SREGS s;
        int     i, j;
        int     Is16Bit=(SND.Flags & SND_DEV_16BIT);

        r.x.eax=0x400;
        int386(0x31, &r, &r);
        IRQ_BaseLo=r.h.dh;
        IRQ_BaseHi=r.h.dl;

        if(SND.IRQ & 0x08)  IRQ_Base=IRQ_BaseHi;
        else                IRQ_Base=IRQ_BaseLo;

        r.x.eax=0x100;
//        r.x.ebx=((MixBufferSize*2)*4)>>4;
        r.x.ebx=((MixBufferSize<<Is16Bit)*4)>>4;
        int386(0x31, &r, &r);

        MixBufferSelector=(r.x.edx & 0xFFFF);
        CurrAddr=(r.x.eax & 0xFFFF)<<4;
        PageTop=(CurrAddr & 0xF0000)+0x10000;

        if((CurrAddr+MixBufferSize*4)>PageTop)
                MixBuffer[0]=(void *)PageTop;
        else
                MixBuffer[0]=(void *)CurrAddr;

        MixBuffer[1]=(void *)((BYTE *)MixBuffer[0]+(MixBufferSize<<Is16Bit));

        r.x.eax = 0x204;
        r.h.bl = IRQ_Base+SND.IRQ;
        int386(0x31, &r, &r);
        OldIRQ_Selector=r.x.ecx;
        OldIRQ_Offset=r.x.edx;

        //for(i=3; i<4; i++) {
        segread(&s);
        if(s.cs!=FP_SEG(IRQ_Handler))
          printf("Beware!! : CS = %d, SEG(IRQ) = %d\n", s.cs, FP_SEG(IRQ_Handler));
        r.x.eax = 0x205;
        r.h.bl  = IRQ_Base+SND.IRQ;
        r.x.ecx = FP_SEG(IRQ_Handler);
        r.x.edx = FP_OFF(IRQ_Handler);
        int386(0x31, &r, &r);
        //}

        }



void    SND_RestoreHandler(void) {
        union REGS r;

        r.x.eax=0x101;
        r.x.edx=OldIRQ_Selector;
        int386(0x31, &r, &r);
        
        r.x.eax = 0x205;
        r.h.bl  = IRQ_Base+SND.IRQ;
        r.x.ecx = OldIRQ_Selector;
        r.x.edx = OldIRQ_Offset;
        int386(0x31, &r, &r);
        }


int     InMixer=0;

void __interrupt IRQ_Handler()
        {
        BYTE    *Vidmem=(BYTE *)0xB8000;
        int     RetCode, MixBufferNum, DMABufferNum, Order,
                Is16Bit=(SND.Flags & SND_DEV_16BIT);


        _disable();
        //Old21=inp(0x21);
        //OldA1=inp(0xA1);
        //outp(0x21, 0xFF);
        //outp(0xA1, 0xFF);

        
        Vidmem[5*160+20*2]++;
        Vidmem[5*160+20*2+1]=14;
        Vidmem[6*160+20*2]='­';
        Vidmem[6*160+20*2+1]=0x3F;
        if(SND.IRQ<8)
          outp(0x21, (inp(0x21) | (1<<SND.IRQ)));
        else
          outp(0xA1, (inp(0xA1) | (1<<(SND.IRQ-8))));

        SND.AcknowledgeIRQ();

        // Call before
        RetCode=SND.CallBefore(PlayFlag);
        MixBufferNum=RetCode & 0x01;
        DMABufferNum=(RetCode>>1) & 0x01;
        Order=(RetCode>>2) & 0x01;

        //if(Order==0) {
        

//        if(!(SND.Flags & SND_DEV_AIDMA))
          dmaStart(SND.DMA, (void *)MixBuffer[DMABufferNum], (MixBufferSize<<Is16Bit), 0x08);
          //DMA_SetDMAC(SND.DMA, (long)MixBuffer[DMABufferNum], (MixBufferSize<<Is16Bit), DMA_MODE_DEMAND, DMA_OPERATION_READ);
        SND.CallAfter(PlayFlag);        

        PlayFlag=((PlayFlag+1) & 0x01);


        outp(0x20, 0x20);
        outp(0xA0, 0x20);
        if(SND.IRQ<8)
          outp(0x21, (inp(0x21) & (~(1<<SND.IRQ))));
        else
          outp(0xA1, (inp(0xA1) & (~(1<<(SND.IRQ-8)))));
        _enable();

        WORD    *DMAdata=(WORD *)MixBuffer[0];
        for(int i=0; i<(SND.ElementLength<<1); i++)
          if(DMAdata[i])
            Vidmem[6*160+30*2]='­';

/*            if(!InMixer) {
                InMixer=1;
//                if(!MixBufferNum)
                  Mix(MixBuffer[MixBufferNum], MixBufferSize, Is16Bit);
                InMixer=0;
                }*/
        //    }

        }

