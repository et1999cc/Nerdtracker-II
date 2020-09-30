
//#include <i86.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
//#include <mem.h>

#include "typedefs.h"
#include "nesess.h"

#define PRINT_SHIT              0
//#define WAP                     1

#define SQUAREWAVE_SCALE        4
#define TRIANGLEWAVE_SCALE      8
#define NOSWAVE_SCALE           0.03125F
#define DPCMWAVE_SCALE          2
#define DPCMWAVE_DELTA          4

#define NFREQ_S                 16

#define ENVELOPE_CONST          4.0F

#define DPCM_DMA_D7TOD0         0

// 15*305*2+7*305+127*3+31*430

typedef struct {
int     DutyCycle;
float   Volume;
int     Decay;
int     Period;
int     Period0;
int     FreqSweepOn;
int     FreqSweepSpeed;
int     FreqSweepDelta;
int     FreqSweepDir;
int     FreqSweepCounter;
int     LCounter;
int     LCounterLast;
int     LCounterRunning;
int     LCounter2;
int     LCounter2Last;
int     LCounter2Running;
int     HoldNote;
int     NoteStopped;
int     ChannelEnabled;
int     LoopedNoise;
int     NoiseFreqVal;
} SynthChannel;

typedef struct {
int     R4010;
int     R4012;
int     R4013;
long    BitAddr;
long    BitLength;
int     Volume;
int     SampleIRQ;
int     Loop;
int     DMAon;
int     DMAturnedoff;
int     FreqVal;
} DPCM_Channel;


inline  int     GetSampleSquareWave(int Chn);
inline  int     GetSampleTriangleWave(int Chn);
inline  int     GetSampleNoiseWave(int Chn);
inline  int     GetSampleDPCMWave(void);

char    Hexxie[17]="0123456789ABCDEF";

void            *UpperPrgPage[4]={NULL,NULL,NULL,NULL};

DPCM_Channel    DPCM_Wave;


float   DPCM_FreqTab[2][16]={{4181.711,4709.927,5264.037,5593.039,
                              6257.946,7046.348,7919.347,8363.423,
                              9419.855,11186.08,12604.03,13982.60,
                              16884.65,21056.15,24857.95,33143.94},
                             {4143.567,4666.965,5216.021,5542.022,
                              6200.864,6982.074,7847.110,8287.136,
                              9333.931,11084.05,12489.06,13855.06,
                              16730.64,20864.09,24631.21,32841.62}};

//      +-------+-----------+------------+
//      | D3-D0 | Frequency | CPU Cycles |
//      +-------+-----------+------------+
//      | %0000 |  4181.711 |      3424  |
//      | %0001 |  4709.927 |      3040  |
//      | %0010 |  5264.037 |      2720  |
//      | %0011 |  5593.039 |      2560  |
//      | %0100 |  6257.946 |      2288  |
//      | %0101 |  7046.348 |      2032  |
//      | %0110 |  7919.347 |      1808  |
//      | %0111 |  8363.423 |      1712  |
//      | %1000 |  9419.855 |      1520  |
//      | %1001 |  11186.08 |      1280  |
//      | %1010 |  12604.03 |      1136  |
//      | %1011 |  13982.60 |      1024  |
//      | %1100 |  16884.65 |       848  |
//      | %1101 |  21056.15 |       680  |
//      | %1110 |  24857.95 |       576  |
//      | %1111 |  33143.94 |       432  |
//      +-------+-----------+------------+

SynthChannel    SynthWave[4];


int     ChannelAmplitude[NUM_CHANNELS];

double          Cycle[NUM_CHANNELS]={0.0, 0.0, 0.0, 0.0, 0.0};
// double          Scale[NUM_A_CHANNELS]={0.0, 0.0, 0.0, 0.0};

int     ChannelActive[NUM_CHANNELS];

float   *Period2FreqTab=NULL;

float   DecayTab[16];

int     PulseState[4][8]={{+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE},
                          {+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE},
                          {+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE},
                          {+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE}};

sBYTE   LCounterTable[32]={5, 127, 10, 1, 20, 2, 40, 3,
                           80, 4, 30, 5, 7, 6, 13, 7,
                           6, 8, 12, 9, 24, 10, 48, 11,
                           96, 12, 36, 13, 8, 14, 16, 15};

void    (*CallInsideMixer)()=NULL;

long    MixingRate=44100;
double  MixingRateR;


int     MixLength;

int     VBlankSpeed=VBLANK_SPEED_NTSC;
int     PlayBackRate=VBLANK_SPEED_NTSC;

double  SoundConst=SOUND_CONST_NTSC;

sBYTE   *NoiseTab=NULL;
int     NoiseSeed=47;

/* const int NoiseFreq[16] =
   {  13982,  6991,  3496,  1748,
        874,   583,   437,   350,
        277,   220,   147,   110,
         77,    54,    28,    14};*/


const float NoiseFreq[16] =
   {  19200*NFREQ_S,  9600*NFREQ_S,  4800*NFREQ_S,  2431*NFREQ_S,
       1189*NFREQ_S,   795*NFREQ_S,   597*NFREQ_S,   477*NFREQ_S,
        378*NFREQ_S,   301*NFREQ_S,   201*NFREQ_S,   156*NFREQ_S,
         79*NFREQ_S,    53*NFREQ_S,    27*NFREQ_S,    14*NFREQ_S};


int     NesessInitialize(void)
        {
        long    i;
        ChannelActive[0]=1;
        ChannelActive[1]=1;
        ChannelActive[2]=1;
        ChannelActive[3]=1;
        ChannelActive[4]=1;

        memset(&SynthWave[0], 0, sizeof(SynthChannel)*4);
        DPCM_Wave.Volume=0x40;

        #ifndef WAP
        if(!SND_SoundSetup()) return(0);
        #endif
        Period2FreqTab=(float *)malloc(sizeof(float)*2048);
        for(i=0; i<2048; i++)
            Period2FreqTab[i]=SoundConst/((float)(i+1));
        for(i=0; i<16; i++)
            DecayTab[i]=ENVELOPE_CONST/((float)(i+1));
        Period2FreqTab[0]=0.0;
        NoiseTab=(sBYTE *)malloc(32768);
        srand(NoiseSeed);
        for(i=0; i<32768; i++)
                NoiseTab[i]=(rand()%256)-128;
        return(1);
        }


int     NesessShutdown(void)
        {
        #ifndef WAP
        SND_SoundShutdown();
        #endif
        free(Period2FreqTab);
        free(NoiseTab);
        return(1);
        }

int     ReadSoundReg(int RegAddr)
        {
        int     i;
        if(RegAddr==0x4015) {
            int Val=0;
            for(i=0; i<4; i++)
                if(SynthWave[i].LCounterRunning) Val|=1<<i;
            if(DPCM_Wave.DMAon)
              Val|=0x10;
            return(Val);
            }
        else if(RegAddr==0x400B) {
            SynthWave[TRIWAV_CHN].LCounter=SynthWave[TRIWAV_CHN].LCounterLast;
            SynthWave[TRIWAV_CHN].LCounter2=SynthWave[TRIWAV_CHN].LCounter2Last;
            }
        return(0);
        }



void    Write2SoundReg(int RegAddr, int Val) {
        char    Hext[17]="0123456789ABCDEF";
        BYTE    *Vidmem=(BYTE *)0xB8000;
        Val&=0xFF;
        //if((RegAddr==0x4015)
        //&& (Val & 0x10))
        //    printf("Oh darn, he _WAS_ right!");
        
/*        if(((RegAddr & 0x40FF)>0x4000) && ((RegAddr & 0x40FF)<0x4015)) {
          char  Hi, Lo;
          int   Reg, Ch;
          if((Val & 0xF)<0xA)
            Lo=(Val & 0xF)+'0';
          else
            Lo=((Val & 0xF)-0xA)+'A';

          if(((Val>>4) & 0xF)<0xA)
            Hi=((Val>>4) & 0xF)+'0';
          else
            Hi=(((Val>>4) & 0xF)-0xA)+'A';


          if((RegAddr>0x4000) && (RegAddr<0x4010)) {
            Reg=RegAddr & 0x03;
            Ch=(RegAddr & 0xF)>>2;
            //Vidmem[20*160+60*2]++;
            //Vidmem[20*160+60*2+1]+=0x07;
            Vidmem[(15+Ch*2)*160+(58+Reg*3)*2]=Hi;
            Vidmem[(15+Ch*2)*160+(58+Reg*3+1)*2]=Lo;
            Vidmem[(15+Ch*2)*160+(58+Reg*3)*2+1]+=0x01;
            Vidmem[(15+Ch*2)*160+(58+Reg*3+1)*2+1]+=0x01;
            Vidmem[(15+Ch*2)*160+(58+Reg*3)*2+1]&=0x0F;
            Vidmem[(15+Ch*2)*160+(58+Reg*3+1)*2+1]&=0x0F;
            Vidmem[(15+Ch*2)*160+54*2]=Ch+'0';
            Vidmem[(16+Ch*2)*160+54*2]=0;
            }
          }*/

        if((RegAddr & 0xFFF8)==0x4000) { // Square wave regs
                int     Reg=(RegAddr & 0x03),
                        Chn=((RegAddr>>2) & 0x01);

                if(Reg==0) { // Square Wave Control Reg #1
                  SynthWave[Chn].HoldNote=((Val>>5) & 0x01);
                  SynthWave[Chn].DutyCycle=((Val>>6) & 0x03);
                  if(Val & 0x10) {
                        SynthWave[Chn].Volume=(Val & 0x0F);
                        SynthWave[Chn].Decay=0;
                        }
                  else
                    SynthWave[Chn].Decay=(Val & 0x0F)+1;
                        
                  }
                if(Reg==1) { // Freq Sweep Register
                  SynthWave[Chn].FreqSweepOn=Val>>7;
                  SynthWave[Chn].FreqSweepSpeed=((Val>>4) & 0x07)+1;
                  SynthWave[Chn].FreqSweepDelta=Val & 0x07;
                  SynthWave[Chn].FreqSweepDir=(Val>>3) & 0x01;
                  SynthWave[Chn].FreqSweepCounter=SynthWave[Chn].FreqSweepSpeed;

//                  SynthWave[Chn].r4001=Val;
                  }

                if(Reg==2) {
                  SynthWave[Chn].Period0=(SynthWave[Chn].Period0 & 0xFF00)+Val;
                  SynthWave[Chn].Period=SynthWave[Chn].Period0;
                  }
                if(Reg==3) {
                  SynthWave[Chn].Period0=(SynthWave[Chn].Period0 & 0x00FF)+((Val & 0x07)<<8);
                  SynthWave[Chn].Period=SynthWave[Chn].Period0;
                  SynthWave[Chn].LCounter=LCounterTable[(Val>>3) & 0x1F];
                  SynthWave[Chn].LCounterLast=SynthWave[Chn].LCounter;
                  SynthWave[Chn].NoteStopped=0;
                  if(SynthWave[Chn].Decay)
                    SynthWave[Chn].Volume=0x0F;
                  }
                }
        else if(RegAddr==0x4008) {
            SynthWave[TRIWAV_CHN].HoldNote=(Val>>7);
            SynthWave[TRIWAV_CHN].LCounter2=(Val & 0x7F);
            SynthWave[TRIWAV_CHN].LCounter2Last=(Val & 0x7F);

            if(!SynthWave[TRIWAV_CHN].HoldNote) {
                SynthWave[TRIWAV_CHN].LCounterRunning=1;
                SynthWave[TRIWAV_CHN].LCounter2Running=1;}
//            if((SynthWave[TRIWAV_CHN].LCounter2==0) && (SynthWave[TRIWAV_CHN].HoldNote==0))
            if(SynthWave[TRIWAV_CHN].LCounter2==0)
              SynthWave[TRIWAV_CHN].NoteStopped=1;
            else
              SynthWave[TRIWAV_CHN].NoteStopped=0;
            }
        else if(RegAddr==0x400A)
            SynthWave[TRIWAV_CHN].Period=(SynthWave[TRIWAV_CHN].Period & 0xFF00)+Val;
        else if(RegAddr==0x400B) {
            SynthWave[TRIWAV_CHN].Period=(SynthWave[TRIWAV_CHN].Period & 0x00FF)+((Val & 0x07)<<8);
            SynthWave[TRIWAV_CHN].LCounter=LCounterTable[(Val>>3) & 0x1F];
            SynthWave[TRIWAV_CHN].LCounterLast=SynthWave[TRIWAV_CHN].LCounter;

            SynthWave[TRIWAV_CHN].LCounter2=SynthWave[TRIWAV_CHN].LCounter2Last;
            if(!SynthWave[TRIWAV_CHN].HoldNote) {
                SynthWave[TRIWAV_CHN].LCounterRunning=1;
                SynthWave[TRIWAV_CHN].LCounter2Running=1;}
            if(SynthWave[TRIWAV_CHN].LCounter2==0)
              SynthWave[TRIWAV_CHN].NoteStopped=1;
            else
              SynthWave[TRIWAV_CHN].NoteStopped=0;
            }
        else if(RegAddr==0x400C) {
                SynthWave[NOSWAV_CHN].HoldNote=((Val>>5) & 0x01);
                if(Val & 0x10) {
                      SynthWave[NOSWAV_CHN].Volume=(Val & 0x0F);
                      SynthWave[NOSWAV_CHN].Decay=0;
                      }
                else  
                  SynthWave[NOSWAV_CHN].Decay=(Val & 0x0F)+1;
                }
        else if(RegAddr==0x400E) {                  
                SynthWave[NOSWAV_CHN].LoopedNoise=(Val>>7);
                SynthWave[NOSWAV_CHN].NoiseFreqVal=(Val & 0xF);
                }
        else if(RegAddr==0x400F) {
            int R0;
            SynthWave[NOSWAV_CHN].LCounter=LCounterTable[(Val>>3) & 0x1F];
            SynthWave[NOSWAV_CHN].LCounterLast=SynthWave[NOSWAV_CHN].LCounter;
            SynthWave[NOSWAV_CHN].NoteStopped=0;
            if(SynthWave[NOSWAV_CHN].Decay)
              SynthWave[NOSWAV_CHN].Volume=0x0F;

            }
        else if(RegAddr==0x4010) {
             //DPCM_Wave.FreqVal=(Val & 0xF);
             //DPCM_Wave.Loop=Val>>6;
             //DPCM_Wave.SampleIRQ=Val>>7;
            DPCM_Wave.R4010=Val;
            }
        else if(RegAddr==0x4011) {
            //DPCM_Wave.Volume=(Val & 0x7F);
            DPCM_Wave.Volume=0x3F;
            //Vidmem[160*10+79*2]++;
            //Vidmem[160*10+79*2+1]++;

            }
            
        else if(RegAddr==0x4012) {
            // DPCM_Wave.BitAddr=(Val<<9);
            // Cycle[DPCM_CHN]=(float)(Val<<9);
            DPCM_Wave.R4012=Val;
            }
            
        else if(RegAddr==0x4013)
            DPCM_Wave.R4013=Val;
            // DPCM_Wave.BitLength=(Val<<7)+1;

        else if(RegAddr==0x4015) {
            SynthWave[SQRWAV1_CHN].ChannelEnabled=(Val & 1);
            SynthWave[SQRWAV2_CHN].ChannelEnabled=((Val>>1) & 1);
            SynthWave[TRIWAV_CHN].ChannelEnabled=((Val>>2) & 1);
            SynthWave[NOSWAV_CHN].ChannelEnabled=((Val>>3) & 1);
//            if((DPCM_Wave.DMAon=((Val>>4) & 1))==1) {
            if(((Val>>4) & 1)==1) {
               if(DPCM_Wave.DMAturnedoff) {
               DPCM_Wave.FreqVal=(DPCM_Wave.R4010 & 0xF);
               DPCM_Wave.Loop=DPCM_Wave.R4010>>6;
               DPCM_Wave.SampleIRQ=DPCM_Wave.R4010>>7;
               DPCM_Wave.BitAddr=(DPCM_Wave.R4012<<9);
               Cycle[DPCM_CHN]=(float)(DPCM_Wave.R4012<<9);
               DPCM_Wave.BitLength=((DPCM_Wave.R4013)<<7);
               DPCM_Wave.DMAon=1;
               }
               DPCM_Wave.DMAturnedoff=0;
               }
            else {
            DPCM_Wave.DMAon=0; // CORRECT?
              DPCM_Wave.DMAturnedoff=1;

              }
            }
        }

float   GetFrequency(int Chn)
        {
        if(Chn<3) {
          if(SynthWave[Chn].Period<8)
            return(0.0);
          else
            if(Period2FreqTab) {
          /* if(Chn==TRIWAV_CHN) {
                 int     P=SynthWave[TRIWAV_CHN].Period;
                return(Period2FreqTab[((P>>1) & 0xF8)+(P & 0xFF)]);}*/

                if((SynthWave[Chn].Period>0) && (SynthWave[Chn].Period<2048)) {
                  if(Chn==TRIWAV_CHN)
                    return(Period2FreqTab[SynthWave[Chn].Period]*0.5);
                  else
                    return(Period2FreqTab[SynthWave[Chn].Period]);
                  }
                else
                  return(0);
                }
          return(0);
          }
        if(Chn==NOSWAV_CHN)
          return((float)NoiseFreq[SynthWave[NOSWAV_CHN].NoiseFreqVal]);
        if(Chn==DPCM_CHN) {
          if(VBlankSpeed==VBLANK_SPEED_PAL) return(DPCM_FreqTab[1][DPCM_Wave.FreqVal]);
        else
          return(DPCM_FreqTab[0][DPCM_Wave.FreqVal]);
          }
        else return(0);
        }

inline  int     GetSample(int Chn)
        {
        if(!ChannelActive[Chn]) return(0);
        if(Chn<4) {
        if((SynthWave[Chn].NoteStopped) || (!SynthWave[Chn].ChannelEnabled))
            return(0);}
        
        if(Chn<2) return(GetSampleSquareWave(Chn));
        else if(Chn==TRIWAV_CHN) return(GetSampleTriangleWave(Chn));
        else if(Chn==NOSWAV_CHN) return(GetSampleNoiseWave(Chn));
        else if(Chn==DPCM_CHN) return(GetSampleDPCMWave());
        else return(0);
        }

inline  int     GetSampleSquareWave(int Chn) {
        int     CurrPulseState=PulseState[SynthWave[Chn].DutyCycle][(((int)(Cycle[Chn]*8)) & 7)];
//        int     CurrPulseState=(((int)Cycle[Chn]) & 1);

//        return(CurrPulseState*SynthWave[Chn].Volume*SQUAREWAVE_SCALE);
        return(CurrPulseState*SynthWave[Chn].Volume);
        }

inline  int     GetSampleTriangleWave(int Chn) {
        int     Counter=(((int)(Cycle[Chn]*32)) & 31);
        int     Amp;


        if(Counter & 0x10)
          Amp=((~(Counter & 0xF)) & 0xF);
        else
          Amp=(Counter & 0xF);
        return((Amp-8)*TRIANGLEWAVE_SCALE);

/*        if(Counter & 0x10)
            Amp=(~(Counter & 0xF));
        else
            Amp=(Counter & 0xF);

        if(Amp>7)
            return((Amp-7)*TRIANGLEWAVE_SCALE);
        else
            return((Amp-8)*TRIANGLEWAVE_SCALE);*/
        }

inline  int     GetSampleNoiseWave(int Chn) {
        int     Index;

        if(SynthWave[Chn].LoopedNoise)
            return((int)NoiseTab[((int)Cycle[Chn])%92]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE);
        else
            return((int)NoiseTab[(((int)Cycle[Chn]) & 32767)]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE);
        }

inline  int     GetSampleDPCMWave(void) {
        int     i;
        BYTE    *UpperPrgPageByte[4];

        UpperPrgPageByte[0]=(BYTE *)UpperPrgPage[0];
        UpperPrgPageByte[1]=(BYTE *)UpperPrgPage[1];
        UpperPrgPageByte[2]=(BYTE *)UpperPrgPage[2];
        UpperPrgPageByte[3]=(BYTE *)UpperPrgPage[3];



        if(!DPCM_Wave.DMAon)
          return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);
          //return(DPCM_Wave.Volume*DPCMWAVE_SCALE);
          


       if((((int)Cycle[DPCM_CHN]) & 0x1FFFF)>DPCM_Wave.BitAddr) {
          int     NumBits=((((int)Cycle[DPCM_CHN]) & 0x1FFFF)-DPCM_Wave.BitAddr) & 3;
          int     ActualNumBits=((((int)Cycle[DPCM_CHN]) & 0x1FFFF)-DPCM_Wave.BitAddr);

          for(i=0; i<NumBits; i++) {
                int     Page=(DPCM_Wave.BitAddr+i)>>15;
                int     ByteAddr=((DPCM_Wave.BitAddr+i)>>3) & 4095;
                int     BitPos=((DPCM_Wave.BitAddr+i) & 7);
                #if(DPCM_DMA_D7TOD0==1)
                int     Bit=(UpperPrgPageByte[Page][ByteAddr]>>(7-BitPos)) & 1;
                #endif
                #if(DPCM_DMA_D7TOD0==0)
                int     Bit=(UpperPrgPageByte[Page][ByteAddr]>>BitPos) & 1;
                #endif
                

                //if(Bit==0) if(DPCM_Wave.Volume>0x00)    DPCM_Wave.Volume--;
                //if(Bit==1) if(DPCM_Wave.Volume<0x7F)    DPCM_Wave.Volume++;

                if(Bit==0) if(DPCM_Wave.Volume>0x00)
                  DPCM_Wave.Volume-=DPCMWAVE_DELTA;
                if(DPCM_Wave.Volume<0)
                  DPCM_Wave.Volume=0;
                if(Bit==1) if(DPCM_Wave.Volume<0x7F)
                  DPCM_Wave.Volume+=DPCMWAVE_DELTA;
                if(DPCM_Wave.Volume>0x7F)
                  DPCM_Wave.Volume=0x7F;

                if((--DPCM_Wave.BitLength)<=0) {
                        if(DPCM_Wave.Loop) {
                                DPCM_Wave.BitAddr=(DPCM_Wave.R4012<<9);
                                DPCM_Wave.BitLength=((DPCM_Wave.R4013<<4)+1)<<3;
                                Cycle[DPCM_CHN]=(float)(DPCM_Wave.R4012<<9);
                                }
                        else {
                          DPCM_Wave.DMAon=0;
                          //if(DPCM_Wave.Volume>=0) DPCM_Wave.Volume&=31;
                          //else DPCM_Wave.Volume=-((-abs(DPCM_Wave.Volume)) & 31);
                          return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);
                          //return(DPCM_Wave.Volume*DPCMWAVE_SCALE);
                            }
                          }
                }
          DPCM_Wave.BitAddr+=ActualNumBits;
          }
        
//        return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);
        return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);
        }
        

inline  void    ProcessWave(int Chn) {
        float   Scale=(float)GetFrequency(Chn)*MixingRateR;
//        if((!ChannelActive[DPCM_CHN]) && (Chn==0)) {
//        Cycle[0]=0.0;
//        Scale=0.0;
//        }

        if(Chn<4) {if(!SynthWave[Chn].NoteStopped) Cycle[Chn]+=Scale;}
        else {
         if(DPCM_Wave.DMAon)
                Cycle[Chn]+=Scale;
         }
        }

inline  void    UpdateSound(void)
        {
        static  int VblankFlag=0;
        int     i, j;
        VblankFlag=((VblankFlag+1) & 0x03);

        if(SynthWave[TRIWAV_CHN].LCounter2Running)
          if(!SynthWave[TRIWAV_CHN].HoldNote)
            if(SynthWave[TRIWAV_CHN].LCounter2)
                if((--SynthWave[TRIWAV_CHN].LCounter2)<=0) {
                    SynthWave[TRIWAV_CHN].NoteStopped=1;
                    }

        if(VblankFlag & 0x01)
          for(i=0; i<2; i++)
            if(SynthWave[i].FreqSweepOn && SynthWave[i].FreqSweepDelta) {
              if((--SynthWave[i].FreqSweepCounter)<=0) {
                SynthWave[i].FreqSweepCounter=SynthWave[i].FreqSweepSpeed;
                if(SynthWave[i].FreqSweepDir)
                  SynthWave[i].Period-=(SynthWave[i].Period>>SynthWave[i].FreqSweepDelta);
                else
                  SynthWave[i].Period+=(SynthWave[i].Period>>SynthWave[i].FreqSweepDelta);
                } // end if(FreqSweepOn)
              } // end for

        if(VblankFlag==0) {
            for(i=0; i<4; i++) {
                if(SynthWave[i].HoldNote!=1)
                  // Really ok to kill this one?
                  if(SynthWave[i].LCounter)
                      if((--SynthWave[i].LCounter)<=0)  {
                          SynthWave[i].NoteStopped=1;
                          SynthWave[i].LCounter=0;
                          }

              if(i!=TRIWAV_CHN) {
                if(SynthWave[i].Decay)
                  SynthWave[i].Volume-=DecayTab[SynthWave[i].Decay-1];
                  // ENVELOPE_CONST/((float)SynthWave[i].Decay);
                if(SynthWave[i].Volume<0) {
                  if(SynthWave[i].HoldNote==1)
                    SynthWave[i].Volume=15;
                  else
                    SynthWave[i].Volume=0;
                  }
                }

              }
            }
        }


inline  void    Mix8Bit(sBYTE *Buffer, int Length, int UnsignedOutput)
        {
        int     i, j, k;
        int     FinalSample;

        for(i=0; i<Length; i++) {
          FinalSample=0;
          for(j=0; j<5; j++) {
                ProcessWave(j);
                FinalSample+=GetSample(j);
                }
//          FinalSample*=100;
          FinalSample=(FinalSample<<6)+(FinalSample<<4);
          if(UnsignedOutput)
            Buffer[i]=((FinalSample>>8) ^ 128);
          else
            Buffer[i]=(FinalSample>>8);
          }
        }

inline  void    Mix16Bit(sWORD *Buffer, int Length, int UnsignedOutput)
        {
        int     i, j, k;
        int     FinalSample;

        for(i=0; i<Length; i++) {
          FinalSample=0;
          for(j=0; j<5; j++) {
                ProcessWave(j);
                FinalSample+=GetSample(j);
                }
//          FinalSample*=100;
          FinalSample=(FinalSample<<6)+(FinalSample<<4);
          if(UnsignedOutput)
            Buffer[i]=(FinalSample ^ 32768);
          else
            Buffer[i]=FinalSample;
          }
        }

// 0156-14055 (Tronds land)

void    Mix(void *Buffer, int Length, int Is16Bit)
        {
        #ifndef WAP
        BYTE    *Vidmem=(BYTE *)0xB8000;
        #endif
        static  int SamplesTilNextUpdate=0;
        static  int SamplesTilNextPlayerCall=0;
        sBYTE   *Buffer8=(sBYTE *)Buffer;
        sWORD   *Buffer16=(sWORD *)Buffer;
        int     SamplesRemainingToBeMixed=Length,
                SamplesMixedSoFar=0,
                MixInThisCall=0;
        int     i, j;
        
        MixingRateR=1/((double)MixingRate);


        {
        int     TempBitAddr=DPCM_Wave.BitAddr,
                TempVolume=DPCM_Wave.Volume,
                TempDMAon=DPCM_Wave.DMAon;
        double  TempCycle=Cycle[DPCM_CHN];

        //for(i=0; i<NUM_A_CHANNELS; i++)
          //ChannelAmplitude[0]=(GetSample(0)*32768)/(SQUAREWAVE_SCALE*15);
          //ChannelAmplitude[1]=(GetSample(1)*32768)/(SQUAREWAVE_SCALE*15);


          if((SynthWave[SQRWAV1_CHN].ChannelEnabled) && (!SynthWave[SQRWAV1_CHN].NoteStopped))
            ChannelAmplitude[SQRWAV1_CHN]=(32768*(SynthWave[SQRWAV1_CHN].Volume+0.5))/15;
          else
            ChannelAmplitude[SQRWAV1_CHN]=0;
          if((SynthWave[SQRWAV2_CHN].ChannelEnabled) && (!SynthWave[SQRWAV2_CHN].NoteStopped))
            ChannelAmplitude[SQRWAV2_CHN]=(32768*(SynthWave[SQRWAV2_CHN].Volume+0.5))/15;
          else
            ChannelAmplitude[SQRWAV2_CHN]=0;


          if((SynthWave[TRIWAV_CHN].ChannelEnabled) && (!SynthWave[TRIWAV_CHN].NoteStopped))
            ChannelAmplitude[TRIWAV_CHN]=32768;
          else
            ChannelAmplitude[TRIWAV_CHN]=0;
          if((SynthWave[NOSWAV_CHN].ChannelEnabled) && (!SynthWave[NOSWAV_CHN].NoteStopped))
            ChannelAmplitude[NOSWAV_CHN]=(32768*(SynthWave[NOSWAV_CHN].Volume+0.5))/15;
          else
            ChannelAmplitude[NOSWAV_CHN]=0;

          //ChannelAmplitude[i]=GetSample(i);
        DPCM_Wave.BitAddr=TempBitAddr;
        DPCM_Wave.Volume=TempVolume;
        DPCM_Wave.DMAon=TempDMAon;
        Cycle[DPCM_CHN]=TempCycle;
        }

        while(SamplesRemainingToBeMixed) {
            if(SamplesTilNextUpdate<SamplesTilNextPlayerCall) {
                if(SamplesTilNextUpdate<SamplesRemainingToBeMixed)
                    MixInThisCall=SamplesTilNextUpdate;
                else
                    MixInThisCall=SamplesRemainingToBeMixed;}
            else {
                if(SamplesTilNextPlayerCall<SamplesRemainingToBeMixed)
                    MixInThisCall=SamplesTilNextPlayerCall;
                else
                    MixInThisCall=SamplesRemainingToBeMixed;}

            if(MixInThisCall) {
                if(Is16Bit==1)
                    #ifndef WAP
                    Mix16Bit(&Buffer16[SamplesMixedSoFar], MixInThisCall, 1);
                    #endif
                    #ifdef WAP
                    Mix16Bit(&Buffer16[SamplesMixedSoFar], MixInThisCall, 0);
                    #endif
                else
                    #ifndef WAP
                    Mix8Bit(&Buffer8[SamplesMixedSoFar], MixInThisCall, 1);
                    #endif
                    #ifdef WAP
                    Mix8Bit(&Buffer8[SamplesMixedSoFar], MixInThisCall, 0);
                    #endif
                } // End Mix
            SamplesMixedSoFar+=MixInThisCall;
            SamplesTilNextUpdate-=MixInThisCall;
            SamplesTilNextPlayerCall-=MixInThisCall;
            SamplesRemainingToBeMixed-=MixInThisCall;

            if(!SamplesTilNextUpdate) {
                UpdateSound();
                SamplesTilNextUpdate=MixingRate/(VBlankSpeed<<2);}
            if(!SamplesTilNextPlayerCall) {
        int     Cyc=(int)Cycle[4];

//        #ifndef WAP
        Vidmem[10*160+69*2]=Hexxie[(DPCM_Wave.R4012 & 0xF)];
        Vidmem[10*160+68*2]=Hexxie[(DPCM_Wave.R4012>>4)];

        Vidmem[10*160+72*2]=Hexxie[(DPCM_Wave.R4013 & 0xF)];
        Vidmem[10*160+71*2]=Hexxie[(DPCM_Wave.R4013>>4)];

        Vidmem[10*160+74*2]=Hexxie[(DPCM_Wave.R4010>>4) & 0xF];
        Vidmem[10*160+75*2]=Hexxie[(DPCM_Wave.R4010 & 0xF)];

        Vidmem[12*160+41*2]=Hexxie[(SynthWave[0].LCounterLast & 0xF)];
        Vidmem[12*160+40*2]=Hexxie[(SynthWave[0].LCounterLast>>4)];

        Vidmem[12*160+44*2]=Hexxie[(SynthWave[1].LCounterLast & 0xF)];
        Vidmem[12*160+43*2]=Hexxie[(SynthWave[1].LCounterLast>>4)];

        Vidmem[12*160+47*2]=Hexxie[(SynthWave[2].LCounterLast & 0xF)];
        Vidmem[12*160+46*2]=Hexxie[(SynthWave[2].LCounterLast>>4)];
                        
        Vidmem[14*160+47*2]=Hexxie[(SynthWave[2].LCounter2Last & 0xF)];
        Vidmem[14*160+46*2]=Hexxie[(SynthWave[2].LCounter2Last>>4)];

        Vidmem[12*160+50*2]=Hexxie[(SynthWave[3].LCounterLast & 0xF)];
        Vidmem[12*160+49*2]=Hexxie[(SynthWave[3].LCounterLast>>4)];

        Vidmem[10*160+66*2]=Hexxie[Cyc & 0xF];
        Vidmem[10*160+65*2]=Hexxie[(Cyc>>4) & 0xF];
        Vidmem[10*160+64*2]=Hexxie[(Cyc>>8) & 0xF];
        Vidmem[10*160+63*2]=Hexxie[(Cyc>>12) & 0xF];
        Vidmem[10*160+62*2]=Hexxie[(Cyc>>16) & 0xF];

        Vidmem[10*160+60*2]=Hexxie[DPCM_Wave.BitAddr & 0xF];
        Vidmem[10*160+59*2]=Hexxie[(DPCM_Wave.BitAddr>>4) & 0xF];
        Vidmem[10*160+58*2]=Hexxie[(DPCM_Wave.BitAddr>>8) & 0xF];
        Vidmem[10*160+57*2]=Hexxie[(DPCM_Wave.BitAddr>>12) & 0xF];
        Vidmem[10*160+56*2]=Hexxie[(DPCM_Wave.BitAddr>>16) & 0xF];

//        Vidmem[23*160+21*2]=Hexxie[(SynthWave[0].r4001 & 0xF)];
//        Vidmem[23*160+20*2]=Hexxie[(SynthWave[0].r4001>>4) & 0xF];

//        Vidmem[23*160+23*2]=Hexxie[(SynthWave[1].r4001 & 0xF)];
//        Vidmem[23*160+22*2]=Hexxie[(SynthWave[1].r4001>>4) & 0xF];

        Vidmem[22*160+23*2]=Hexxie[SynthWave[0].Period & 0xF];
        Vidmem[22*160+22*2]=Hexxie[(SynthWave[0].Period>>4) & 0xF];
        Vidmem[22*160+21*2]=Hexxie[(SynthWave[0].Period>>8) & 0xF];
        Vidmem[22*160+20*2]=Hexxie[(SynthWave[0].Period>>12) & 0xF];

        Vidmem[21*160+25*2]=Hexxie[SynthWave[0].Decay & 0xF];

        Vidmem[19*160+71*2]=Hexxie[DPCM_Wave.Volume & 0xF];
        Vidmem[19*160+70*2]=Hexxie[(DPCM_Wave.Volume>>4) & 0xF];

        Vidmem[21*160+71*2]=Hexxie[((int)SynthWave[NOSWAV_CHN].Volume) & 0xF];
//        #endif
                if(CallInsideMixer) CallInsideMixer();
                SamplesTilNextPlayerCall=MixingRate/PlayBackRate;}
                
            }
        }
