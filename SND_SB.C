#include <i86.h>
#include <stdlib.h>

#include "typedefs.h"
#include "dma.h"
#include "nesess.h"

#include "snd_sb.h"


#define SB_DSP_RESETPORT        0x06   // Write only
#define SB_DSP_READDATAPORT     0x0A   // Read only
#define SB_DSP_WRITEDATAPORT    0x0C   // Write
#define SB_DSP_DATAAVAILABLE    0x0E   // Read only

#define SB_DSPCOMM_DMAMODEDACLO 0x14
#define SB_DSPCOMM_DMAMODEDACHI 0x91
#define SB_DSPCOMM_SETTIMECONST 0x40
#define SB_DSPCOMM_SETBLKSIZE   0x48
#define SB_DSPCOMM_SPK_ON       0xD1
#define SB_DSPCOMM_SPK_OFF      0xD3
#define SB_DSPCOMM_HALTDMA      0xD0
#define SB_DSPCOMM_CONTDMA      0xD4


int     SB_BaseAddr=0x220,
        SB_IRQ=7,
        SB_DMA=1;

SND_DeviceTable *SB;

int     SB_Initialize(void)
        {
        int     i;
        outp(SB_BaseAddr+SB_DSP_RESETPORT, 1);

        // Wait 3 microseconds!
        delay(3);

        outp(SB_BaseAddr+SB_DSP_RESETPORT, 0);
        while(!(inp(SB_BaseAddr+SB_DSP_DATAAVAILABLE) & 0x80));

          for(i=0; i<100; i++) {
            if(inp(SB_BaseAddr+SB_DSP_READDATAPORT)==0xAA) {
              SB_WriteDSP(SB_DSPCOMM_SPK_ON);
              return(1);}
            }
          return(0);
        }

void    SB_WriteDSP(int x)
        {
        while(inp(SB_BaseAddr+SB_DSP_WRITEDATAPORT) & 0x80);
        outp(SB_BaseAddr+SB_DSP_WRITEDATAPORT, x);
        }

int     SB_ReadDSP(void)
        {
        while(!inp(SB_BaseAddr+SB_DSP_DATAAVAILABLE) & 0x80);
        return(inp(SB_BaseAddr+SB_DSP_READDATAPORT));
        }

void    SB_AcknowledgeIRQ(void)
        {
        inp(SB_BaseAddr+SB_DSP_DATAAVAILABLE);
        }


void    SB_ReceiveDMA(long Addr, long DataLength, long Rate)
        {
        long    TimeConst;
        //SB_WriteDSP(SB_DSPCOMM_SPK_ON);
        //SB_WriteDSP(SB_DSPCOMM_SPK_OFF);

        if(Rate<21739)
          TimeConst=(256-(1000000/Rate));
        else
          TimeConst=(65536-(256000000/Rate))>>8;

        SB_WriteDSP(SB_DSPCOMM_SETTIMECONST);
        SB_WriteDSP(TimeConst);
        if(Rate<21739)
          SB_WriteDSP(SB_DSPCOMM_DMAMODEDACLO);
        else
          SB_WriteDSP(SB_DSPCOMM_DMAMODEDACHI);
        SB_WriteDSP((DataLength-1) & 0xFF);
        SB_WriteDSP((DataLength-1)>>8);
        // Service Interrupt
        // Restore ISR
        }

int     SB_CallBefore(int CurrentBuffNum)
        {
        long    TimeConst,
                Rate=(*SB).MixingRate,
                DataLength=(*SB).ElementLength;
        int     MixBuffNum=(CurrentBuffNum+1) & 0x01,
                DMABuffNum=CurrentBuffNum,
                Order=0;

        if(Rate<21739)
          TimeConst=(256-(1000000/Rate));
        else
          TimeConst=(65536-(256000000/Rate))>>8;

        SB_WriteDSP(SB_DSPCOMM_SETTIMECONST);
        SB_WriteDSP(TimeConst);
        if(Rate<21739)
          SB_WriteDSP(SB_DSPCOMM_DMAMODEDACLO);
        else
          SB_WriteDSP(SB_DSPCOMM_DMAMODEDACHI);
        SB_WriteDSP((DataLength-1) & 0xFF);
        SB_WriteDSP((DataLength-1)>>8);
        // Service Interrupt
        // Restore ISR
        return(MixBuffNum+(DMABuffNum<<1)+(Order<<2));
        }

int     SB_CallAfter(int CurrentBuffNum)
        {
        return(0);
        }

void    SB_StartOutput(void)
        {
        SB_CallBefore(0);
        }

int     SB_AutoDetect(SND_DeviceTable *DevTab)
        {
        int     i, j;
        char    *EnvVar;
        if((EnvVar=getenv("BLASTER"))==0) {
        printf("BLASTER environment variable not found!");
          return(0);
        }
        i=0;
        while(toupper(EnvVar[i])!='A') i++;
        SB_BaseAddr=0x200 | ((int)(EnvVar[i+2]-'0')<<4);
        while(toupper(EnvVar[i])!='I') i++;
        SB_IRQ=EnvVar[i+1]-'0';
        while(toupper(EnvVar[i])!='D') i++;
        SB_DMA=EnvVar[i+1]-'0';
        (*DevTab).BaseAddr=SB_BaseAddr;
        (*DevTab).IRQ=SB_IRQ;
        (*DevTab).DMA=SB_DMA;
        (*DevTab).MaxMixingRate=21700;
        (*DevTab).Flags=SND_DEV_UNSIGNEDOUTPUT;

        (*DevTab).Initialize=SB_Initialize;
        (*DevTab).AcknowledgeIRQ=SB_AcknowledgeIRQ;
        (*DevTab).ReceiveDMA=SB_ReceiveDMA;
        (*DevTab).CallBefore=SB_CallBefore;
        (*DevTab).CallAfter=SB_CallAfter;
        (*DevTab).StartOutput=SB_StartOutput;
        SB=DevTab;
        printf("What the???!!!\n");
        return(1);
        }
