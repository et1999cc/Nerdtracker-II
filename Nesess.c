
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


//#define SQUAREWAVE_SCALE        220
//#define TRIANGLEWAVE_SCALE      330
//#define TRIANGLEWAVE_SCALE      356
//#define NOSWAVE_SCALE           1.75F
//#define DPCMWAVE_SCALE          315
//#define DPCMWAVE_SCALE          350

#define SQUAREWAVE_SCALE        4

#define SQW_SCALE_VRC6          4

#define TRIANGLEWAVE_SCALE      8
#define NOSWAVE_SCALE           0.03125F
#define DPCMWAVE_SCALE          4
#define DPCMWAVE_DELTA          2


#define NFREQ_S                 16

#define ENVELOPE_CONST          4.0F

#define DPCM_DMA_D7TOD0         0

// 15*305*2+7*305+127*3+31*430

typedef struct {
int     DutyCycle;
float   Volume;
float	DecayCounter;
int		NoDecay;
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

int     Gate;
int     Phase;
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


static  int     GetSampleSquareWave(int Chn);
static  int     GetIntSampleSquareWave(int Chn);
static  int     GetSampleTriangleWave(int Chn);
static  int     GetSampleNoiseWave(int Chn);
static  int     GetSampleDPCMWave(void);

char    Hexxie[17]="0123456789ABCDEF";

void            *UpperPrgPage[4]={NULL,NULL,NULL,NULL};

DPCM_Channel    DPCM_Wave;


/* float   DPCM_FreqTab[16]={4181.711,4709.927,5264.037,5593.039,
                          6257.946,7046.348,7919.347,8363.423,
                          9419.855,11186.08,12604.03,13982.60,
                          16884.65,21056.15,24857.95,33143.94};*/

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

SynthChannel    SynthWave[NUM_CHANNELS];


int     ChannelAmplitude[NUM_CHANNELS];

double          Cycle[NUM_CHANNELS]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
// double          Scale[NUM_A_CHANNELS]={0.0, 0.0, 0.0, 0.0};

int     ChannelActive[NUM_CHANNELS];

float   *Period2FreqTab[2]={NULL,NULL};

int     PulseState[4][8]={{+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE},
                          {+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE},
                          {+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE},
                          {+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE}};

int     PulseStateVRC6[8][16]={{+1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,-1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6},
                               {+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQW_SCALE_VRC6,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQUAREWAVE_SCALE,+1*SQW_SCALE_VRC6,   -1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6,-1*SQW_SCALE_VRC6}};

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

int		SoundConstIsPAL=0;

sBYTE   *NoiseTab=NULL;
int     NoiseSeed=47;

int     EmulateVRC6=0;

int		NumFramesPlayed;


const float NoiseFreq[16] =
   {447443.15, 223721.58, 111860.79, 55930.39,
    27965.20, 18643.47, 13982.60, 11186.08,
    8860.26, 7046.35, 4709.93, 3523.17,
    2348.78, 1761.59, 879.93, 874.77};


/*const float NoiseFreq[16] =
   {  19200*NFREQ_S,  9600*NFREQ_S,  4800*NFREQ_S,  2431*NFREQ_S,
       1189*NFREQ_S,   795*NFREQ_S,   597*NFREQ_S,   477*NFREQ_S,
        378*NFREQ_S,   301*NFREQ_S,   201*NFREQ_S,   156*NFREQ_S,
         79*NFREQ_S,    53*NFREQ_S,    27*NFREQ_S,    14*NFREQ_S};*/



double   OldFilter[32]={
-0.002176373,
-0.006216011,
-0.006703078,
-0.0000000000000939353,
0.01181114,
0.02088058,
0.01813461,
-0.0000000000000388325,
-0.02692538,
-0.04676893,
-0.04134074,
-0.000000000000765252,
0.07284433,
0.1571044,
0.2243555,
0.25,
// 0.25,
0.2243555,
0.1571044,
0.07284433,
0.000000000000109197,
-0.04134074,
-0.04676893,
-0.02692538,
0.0000000000000130274,
0.01813461,
0.02088058,
0.01181114,
0.0000000000000564271,
-0.006703078,
-0.006216011,
-0.002176373,

0,
};

double   Filter[32]={
0.0625,
0.06189954,
0.06012124,
0.05723343,
0.05334709,
0.04861157,
0.04320886,
0.03734657,
0.03125,
0.02515343,
0.01929114,
0.01388843,
0.009152913,
0.005266575,
0.002378765,
0.00060046,
0,
0.00060046,
0.002378765,
0.005266575,
0.009152913,
0.01388843,
0.01929114,
0.02515343,
0.03125,
0.03734657,
0.04320886,
0.04861157,
0.05334709,
0.05723343,
0.06012124,
0.06189954,

};

double   BiceFilter[32]={
-0.030,
-0.025,
-0.01,
0.02,
0.03,
0.03,
0.02,
-0.01,
-0.030,
-0.05,
-0.045,
-0.02,
0.025,
0.9,
0.16,
0.2,

0.22,

0.2,
0.16,
0.9,
0.025,
-0.02,
-0.045,
-0.05,
-0.030,
-0.01,
0.02,
0.03,
0.03,
0.02,
-0.01,
-0.025
// -0.030,
};



/* emulate a 15-bit shift register, with
** feedback taps at bits 8 and 15
*/
int shift_register15(int DoReset)
{
   static int sreg = 1;
   int bit1, bit8, bit15;

   if(DoReset)
      sreg=1;
   bit15 = sreg & 1;
   bit8 = (sreg & 0x100) >> 8;
   bit1 = bit15 ^ bit8;
   sreg >>= 1;
   sreg |= (bit1 << 14);
   return bit15;
}




double	CutOffFreq=18000.0,
		FilterA,
		FilterB;
int		DCoffs=00000;

double	a0,a1,a2,b0,b1;

int     NesessInitialize(void)
        {
        long    i;
        ChannelActive[0]=1;
        ChannelActive[1]=1;
        ChannelActive[2]=1;
        ChannelActive[3]=1;
        ChannelActive[4]=1;

        ChannelActive[5]=1;
        ChannelActive[6]=1;
        ChannelActive[7]=1;


        memset(&SynthWave[0], 0, sizeof(SynthChannel)*4);
        DPCM_Wave.Volume=0x40;

        #ifndef WAP
        if(!SND_SoundSetup()) return(0);
        #endif
        Period2FreqTab[0]=(float *)malloc(sizeof(float)*2048);
		Period2FreqTab[1]=(float *)malloc(sizeof(float)*2048);
        for(i=0; i<2048; i++)
            Period2FreqTab[0][i]=SOUND_CONST_NTSC/((float)(i+1));
        for(i=0; i<2048; i++)
            Period2FreqTab[1][i]=SOUND_CONST_PAL/((float)(i+1));

		NoiseTab=(sBYTE *)malloc(32768);
        //srand(NoiseSeed);
        for(i=0; i<32768; i++) {
                int Temp=(shift_register15(i==0)<<8);
				if(Temp>=256)
					Temp=255;
				NoiseTab[i]=Temp-128;
				}
				//((rand()%256) & 0xFF)-128;
        return(1);
        }


int     NesessShutdown(void)
        {
        #ifndef WAP
        SND_SoundShutdown();
        #endif
        free(Period2FreqTab[0]);
		free(Period2FreqTab[1]);
        free(NoiseTab);
        return(1);
        }

int     ReadSoundReg(int RegAddr)
        {
        int     i;
        if(RegAddr==0x4015) {
            int Val=0;
            for(i=0; i<4; i++)
                //if(SynthWave[i].LCounterRunning)
				if((!SynthWave[i].NoteStopped) && (SynthWave[i].ChannelEnabled))
					Val|=1<<i;
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
        Val&=0xFF;

        if((RegAddr & 0xFFF8)==0x4000) { // Square wave regs
                int     Reg=(RegAddr & 0x03),
                        Chn=((RegAddr>>2) & 0x01);

                if(Reg==0) { // Square Wave Control Reg #1
                  SynthWave[Chn].HoldNote=((Val>>5) & 0x01);
					//SynthWave[Chn].HoldNote=1;
                  SynthWave[Chn].DutyCycle=((Val>>6) & 0x03);

				  SynthWave[Chn].NoDecay=(Val>>4) & 0x1;

				  if(Val & 0x10) {
                        SynthWave[Chn].Volume=(Val & 0x0F);
                        //SynthWave[Chn].Decay=0;
                        }
                  else {
                    SynthWave[Chn].Decay=(Val & 0x0F)+1;
					//SynthWave[Chn].Volume=0x0F;
					}
                  }
                if(Reg==1) { // Freq Sweep Register
                  SynthWave[Chn].FreqSweepOn=Val>>7;
                  SynthWave[Chn].FreqSweepSpeed=((Val>>4) & 0x07)+1;
                  SynthWave[Chn].FreqSweepDelta=Val & 0x07;
                  SynthWave[Chn].FreqSweepDir=(Val>>3) & 0x01;
                  SynthWave[Chn].FreqSweepCounter=SynthWave[Chn].FreqSweepSpeed;
                  }

                if(Reg==2) {
                  SynthWave[Chn].Period0=(SynthWave[Chn].Period0 & 0xFF00)+Val;
                  SynthWave[Chn].Period=SynthWave[Chn].Period0;
                  }
                if(Reg==3) {
                  	// Reset phase when REG3 is written
					Cycle[Chn]=0.0;
				  SynthWave[Chn].Period0=(SynthWave[Chn].Period0 & 0x00FF)+((Val & 0x07)<<8);
                  SynthWave[Chn].Period=SynthWave[Chn].Period0;
                  SynthWave[Chn].LCounter=LCounterTable[(Val>>3) & 0x1F];

				  //SynthWave[Chn].LCounter=127;

                  SynthWave[Chn].LCounterLast=SynthWave[Chn].LCounter;
                  SynthWave[Chn].NoteStopped=0;
                  //if(SynthWave[Chn].Decay)
                  //  SynthWave[Chn].Volume=0x0F;
				  SynthWave[Chn].DecayCounter=0xF;
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
                //Cycle[NOSWAV_CHN]=0.0;
				SynthWave[NOSWAV_CHN].HoldNote=((Val>>5) & 0x01);

				SynthWave[NOSWAV_CHN].NoDecay=(Val>>4) & 0x1;

				if(Val & 0x10) {
                      SynthWave[NOSWAV_CHN].Volume=(Val & 0x0F);
                      //SynthWave[NOSWAV_CHN].Decay=0;
                      }
                else
                  SynthWave[NOSWAV_CHN].Decay=(Val & 0x0F)+1;
                }
        //else if(RegAddr==0x400D) Cycle[NOSWAV_CHN]=0.0;
		else if(RegAddr==0x400E) {
            //Cycle[NOSWAV_CHN]=0.0;
			SynthWave[NOSWAV_CHN].LoopedNoise=(Val>>7);
                SynthWave[NOSWAV_CHN].NoiseFreqVal=(Val & 0xF);
                }
        else if(RegAddr==0x400F) {
            //Cycle[NOSWAV_CHN]=0.0;
			SynthWave[NOSWAV_CHN].LCounter=LCounterTable[(Val>>3) & 0x1F];
            SynthWave[NOSWAV_CHN].LCounterLast=SynthWave[NOSWAV_CHN].LCounter;
            SynthWave[NOSWAV_CHN].NoteStopped=0;
            //if(SynthWave[NOSWAV_CHN].Decay)
            //  SynthWave[NOSWAV_CHN].Volume=0x0F;
			SynthWave[NOSWAV_CHN].DecayCounter=0x0F;
            }
        else if(RegAddr==0x4010) {
//             DPCM_Wave.FreqVal=(Val & 0xF);
//             DPCM_Wave.Loop=Val>>6;
//             DPCM_Wave.SampleIRQ=Val>>7;
            DPCM_Wave.R4010=Val;
            }
        else if(RegAddr==0x4011) {
            DPCM_Wave.Volume=(Val & 0x7F);
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
			//if(!SynthWave[SQRWAV1_CHN].ChannelEnabled)
			//	Cycle[SQRWAV1_CHN]=0.0;
			//if(!SynthWave[SQRWAV2_CHN].ChannelEnabled)
			//	Cycle[SQRWAV2_CHN]=0.0;
			//if(!SynthWave[TRIWAV_CHN].ChannelEnabled)
			//	Cycle[TRIWAV_CHN]=0.0;
			//if(!SynthWave[NOSWAV_CHN].ChannelEnabled)
			//	Cycle[NOSWAV_CHN]=0.0;

//            if((DPCM_Wave.DMAon=((Val>>4) & 1))==1) {
            if(((Val>>4) & 1)==1) {
               if(DPCM_Wave.DMAturnedoff) {
               DPCM_Wave.FreqVal=(DPCM_Wave.R4010 & 0xF);
               DPCM_Wave.Loop=DPCM_Wave.R4010>>6;
               DPCM_Wave.SampleIRQ=DPCM_Wave.R4010>>7;
               DPCM_Wave.BitAddr=(DPCM_Wave.R4012<<9);
               Cycle[DPCM_CHN]=(float)(DPCM_Wave.R4012<<9);
               DPCM_Wave.BitLength=(((DPCM_Wave.R4013<<4)+1)<<3);
               DPCM_Wave.DMAon=1;
               }

               DPCM_Wave.DMAturnedoff=0;
               }
            else {
               DPCM_Wave.DMAon=0; // CORRECT?
               DPCM_Wave.DMAturnedoff=1;
               }
            }

        if(EmulateVRC6) {
          if(((RegAddr & 0xFFFC)==0x9000) || ((RegAddr  & 0xFFFC)==0xA000)) {
            int Reg, Chn;
            Reg=RegAddr & 3;

            if((RegAddr & 0xFFFC)==0x9000)
              Chn=VRC6_SQRW1_CHN;
            else
              Chn=VRC6_SQRW2_CHN;

            if(Reg==0) {
              SynthWave[Chn].DutyCycle=(Val>>4) & 7;
              SynthWave[Chn].Volume=Val & 0xF;
              SynthWave[Chn].Gate=Val>>7;
              }
            if(Reg==1) {
              SynthWave[Chn].Period0=(SynthWave[Chn].Period0 & 0xFF00)+Val;
              SynthWave[Chn].Period=SynthWave[Chn].Period0;
              }
            if(Reg==2) {
              SynthWave[Chn].Period0=(SynthWave[Chn].Period0 & 0x00FF)+((Val & 0x0F)<<8);
              SynthWave[Chn].Period=SynthWave[Chn].Period0;
              SynthWave[Chn].ChannelEnabled=Val>>7;
              }
            }
          if((RegAddr & 0xFFFC)==0xB000) {
            int Reg=RegAddr & 3;

            if(Reg==0)
              SynthWave[VRC6_SAW_CHN].Phase=Val & 0x3F;
            if(Reg==1) {
              SynthWave[VRC6_SAW_CHN].Period0=(SynthWave[VRC6_SAW_CHN].Period0 & 0xFF00)+Val;
              SynthWave[VRC6_SAW_CHN].Period=SynthWave[VRC6_SAW_CHN].Period0;
              }
            if(Reg==2) {
              SynthWave[VRC6_SAW_CHN].Period0=(SynthWave[VRC6_SAW_CHN].Period0 & 0x00FF)+((Val & 0x0F)<<8);
              SynthWave[VRC6_SAW_CHN].Period=SynthWave[VRC6_SAW_CHN].Period0;
              SynthWave[VRC6_SAW_CHN].ChannelEnabled=Val>>7;
			  //SynthWave[VRC6_SAW_CHN].ChannelEnabled=1;
			  SynthWave[VRC6_SAW_CHN].NoteStopped=0;
              }
            }
          } // end if(EmulateVRC6)
        }

float   GetFrequency(int Chn)
        {
        if(Chn<3) {
          if(SynthWave[Chn].Period<8)
            return(0.0);
          else
            if(Period2FreqTab) {

                if((SynthWave[Chn].Period>0) && (SynthWave[Chn].Period<2048)) {
                  if(Chn==TRIWAV_CHN)
                    return(Period2FreqTab[SoundConstIsPAL][SynthWave[Chn].Period]*0.5);
                  else
                    return(Period2FreqTab[SoundConstIsPAL][SynthWave[Chn].Period]);
                  }
                else
					//return(Period2FreqTab[SoundConstIsPAL][2047]);
                  return(0);
                }
          return(0);
          }
        if(Chn==NOSWAV_CHN)
          return((float)NoiseFreq[SynthWave[NOSWAV_CHN].NoiseFreqVal]);
//        if(Chn==DPCM_CHN) return(DPCM_FreqTab[DPCM_Wave.FreqVal]);
        if(Chn==DPCM_CHN) {
          if(VBlankSpeed==VBLANK_SPEED_PAL)
			  return(DPCM_FreqTab[0][DPCM_Wave.FreqVal]);
          else
			  return(DPCM_FreqTab[0][DPCM_Wave.FreqVal]);
          }
        if(EmulateVRC6) {
          if((Chn==VRC6_SQRW1_CHN) || (Chn==VRC6_SQRW2_CHN)) {
            return(1789772.7272/(float)((SynthWave[Chn].Period+1)*16));
            }
          if(Chn==VRC6_SAW_CHN) {
            // return(1789772.7272/(float)((SynthWave[Chn].Period+1)*14));
            return(1789772.7272/(float)((SynthWave[Chn].Period+1)*2));
            }
          }
        else return(0);
        }

static  int     GetSample(int Chn)
        {
        if(!ChannelActive[Chn]) return(0);
        if(Chn<4) {
        //if((SynthWave[Chn].NoteStopped) || (!SynthWave[Chn].ChannelEnabled))
        //    return(0);}

		// Makes the channels amp remain at the last value
		// when stopped.
		if((!SynthWave[Chn].ChannelEnabled) || SynthWave[Chn].NoteStopped)
            return(0);
		}

        if(Chn<2)
            return(GetSampleSquareWave(Chn));
			//return(GetIntSampleSquareWave(Chn));

        else if(Chn==TRIWAV_CHN) return(GetSampleTriangleWave(Chn));
        else if(Chn==NOSWAV_CHN) return(GetSampleNoiseWave(Chn));
        else if(Chn==DPCM_CHN) return(GetSampleDPCMWave());
        else if(Chn==VRC6_SQRW1_CHN)
               return(GetSampleSquareWaveVRC6(Chn));
        else if(Chn==VRC6_SQRW2_CHN)
               return(GetSampleSquareWaveVRC6(Chn));
        else if(Chn==VRC6_SAW_CHN)
               return(GetSampleSawToothWaveVRC6(Chn));
        else return(0);
        }

static  int     GetSampleSquareWave(int Chn) {
        int     CurrPulseState=PulseState[SynthWave[Chn].DutyCycle][(((int)(Cycle[Chn]*8)) & 7)];

        //return(CurrPulseState*SynthWave[Chn].Volume);

		//return(CurrPulseState*15);

		if(SynthWave[Chn].NoDecay)
			return(CurrPulseState*SynthWave[Chn].Volume);
		else
			return(CurrPulseState*SynthWave[Chn].DecayCounter);
        }

static  int     GetIntSampleSquareWave(int Chn) {
        float   PulseState0=PulseState[SynthWave[Chn].DutyCycle][(((int)(Cycle[Chn]*8)) & 7)],
                PulseState1=PulseState[SynthWave[Chn].DutyCycle][(((int)(Cycle[Chn]*8)+1) & 7)];
        float   FracPart=(Cycle[Chn]*8)-(float)((int)(Cycle[Chn]*8));

        int     CurrPulseState;

        //if((Cycle[Chn]*8)-(float)((int)(Cycle[Chn]*8))>0.5)
        //  CurrPulseState=PulseState[2][((((int)(Cycle[Chn]*8))+1) & 7)];
          //CurrPulseState=PulseState[SynthWave[Chn].DutyCycle][((((int)(Cycle[Chn]*8))+1) & 7)];
        //else
        //  CurrPulseState=PulseState[2][((((int)(Cycle[Chn]*8))) & 7)];
        //  //CurrPulseState=PulseState[SynthWave[Chn].DutyCycle][((((int)(Cycle[Chn]*8))) & 7)];

        //return(CurrPulseState*SynthWave[Chn].Volume);


        return((PulseState1*FracPart+PulseState0*(1.0-FracPart))*SynthWave[Chn].Volume);
        }



static  int     GetSampleSquareWaveVRC6(int Chn) {
        int     CurrPulseState=PulseStateVRC6[SynthWave[Chn].DutyCycle][(((int)(Cycle[Chn]*16)) & 0xF)];
        if(SynthWave[Chn].Gate)
          return(SynthWave[Chn].Volume);
        else
          return(CurrPulseState*SynthWave[Chn].Volume);
		}


static  int     GetSampleSawToothWaveVRC6(int Chn) {
        int     Acc=(((int)Cycle[Chn])%7)*SynthWave[VRC6_SAW_CHN].Phase;
		return(((Acc & 0xFF)>>3)-16);
		}


static  int     GetSampleTriangleWave(int Chn) {
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

static  int     GetIntSampleNoiseWave(int Chn) {
        int     Index;

		double CycFrac=Cycle[Chn]-(int)Cycle[Chn];
        if(SynthWave[Chn].LoopedNoise) {
			int Samp0=NoiseTab[((int)Cycle[Chn])%92]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE;
			int Samp1=NoiseTab[(((int)Cycle[Chn])+1)%92]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE;
			int Samp=(int)((double)Samp0*(1.0-CycFrac)+(double)Samp1*CycFrac);
			return(Samp);
			}
            //return((int)NoiseTab[((int)Cycle[Chn])%92]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE);
        else {
            int Samp0=NoiseTab[((int)Cycle[Chn]) & 32767]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE;
			int Samp1=NoiseTab[(((int)Cycle[Chn])+1) & 32767]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE;
			int Samp=(int)((double)Samp0*(1.0-CycFrac)+(double)Samp1*CycFrac);
			return(Samp);
			}
        }


static  int     GetSampleNoiseWave(int Chn) {
        static	int     Index[16], i, Sample=0, vol;
		float	c=Cycle[Chn],
				cAdd=(float)NoiseFreq[SynthWave[NOSWAV_CHN].NoiseFreqVal]*MixingRateR;

		if(SynthWave[NOSWAV_CHN].NoDecay)
			vol=SynthWave[NOSWAV_CHN].Volume;
		else
			vol=SynthWave[NOSWAV_CHN].DecayCounter;

		for(i=0; i<16; i++) {
			if(SynthWave[NOSWAV_CHN].LoopedNoise)
				//Index[i]=((int)c)%92;
				Index[i]=(int)(c-92*(int)(c*0.0108695652));
			else
				Index[i]=((int)c) & 32767;
			c+=cAdd*0.0625;
			Sample+=NoiseTab[Index[i]];
			}

		//if(SynthWave[Chn].LoopedNoise)
		//	return((int)NoiseTab[((int)Cycle[Chn])%92]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE);
            //return((int)NoiseTab[((int)Cycle[Chn])%92]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE);
        //else
        //    return((int)NoiseTab[(((int)Cycle[Chn]) & 32767)]*SynthWave[NOSWAV_CHN].Volume*NOSWAVE_SCALE);

		//Sample*=SynthWave[NOSWAV_CHN].Volume*(0.0625*NOSWAVE_SCALE);
		Sample*=vol*(0.0625*NOSWAVE_SCALE);

		return(Sample);
		}

static  int     GetSampleDPCMWave(void) {
        int     i;
        BYTE    *UpperPrgPageByte[4];

        UpperPrgPageByte[0]=(BYTE *)UpperPrgPage[0];
        UpperPrgPageByte[1]=(BYTE *)UpperPrgPage[1];
        UpperPrgPageByte[2]=(BYTE *)UpperPrgPage[2];
        UpperPrgPageByte[3]=(BYTE *)UpperPrgPage[3];



        if(!DPCM_Wave.DMAon)
//          return((DPCM_Wave.Volume & 0x7F)*DPCMWAVE_SCALE);
          return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);




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


                //if(Bit==0) if(DPCM_Wave.Volume>0)       DPCM_Wave.Volume--;
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
//                          return((DPCM_Wave.Volume & 0x7F)*DPCMWAVE_SCALE);
                          return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);
                            }
                          }
                }
          DPCM_Wave.BitAddr+=ActualNumBits;
          }

//        return((DPCM_Wave.Volume & 0x7F)*DPCMWAVE_SCALE);
        return(((DPCM_Wave.Volume & 0x7F)-0x40)*DPCMWAVE_SCALE);
        }


static  void    ProcessWave(int Chn) {
        float   Scale=(float)GetFrequency(Chn)*MixingRateR;
//        if((!ChannelActive[DPCM_CHN]) && (Chn==0)) {
//        Cycle[0]=0.0;
//        Scale=0.0;
//        }

        if(Chn!=4) {
			//if((!SynthWave[Chn].NoteStopped) && SynthWave[Chn].ChannelEnabled)

				Cycle[Chn]+=Scale;
			}
        else {
         if(DPCM_Wave.DMAon)
                Cycle[Chn]+=Scale;
         }
        }

static  void    UpdateSound(void)
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
                  // Really ok to kill this one? - Guess not! (fucks up triwav
                  //                                           in Ironsword)
                  if(SynthWave[i].LCounter)
                      if((--SynthWave[i].LCounter)<=0)  {
                          SynthWave[i].NoteStopped=1;
                          SynthWave[i].LCounter=0;
                          }

              if(i!=TRIWAV_CHN) {
                if(SynthWave[i].Decay)
                  //SynthWave[i].Volume-=ENVELOPE_CONST/((float)SynthWave[i].Decay);
				  SynthWave[i].DecayCounter-=ENVELOPE_CONST/((float)SynthWave[i].Decay);
				//if(SynthWave[i].Volume<0) {
				if(SynthWave[i].DecayCounter<0) {
                  if(SynthWave[i].HoldNote==1)
                    //SynthWave[i].Volume=15;
					SynthWave[i].DecayCounter=15;
                  else
                    //SynthWave[i].Volume=0;
					SynthWave[i].DecayCounter=0;
                  }
                }
              }
            }
        }


static  void    Mix8Bit(sBYTE *Buffer, int Length, int UnsignedOutput)
        {
        int     i, j, k;
        int     FinalSample;

        for(i=0; i<Length; i++) {
          FinalSample=0;
          for(j=0; j<5; j++) {
                ProcessWave(j);
                FinalSample+=GetSample(j);
                }
          FinalSample<<=6;
          if(UnsignedOutput)
            Buffer[i]=((FinalSample>>8) ^ 128);
          else
            Buffer[i]=(FinalSample>>8);
          }
        }

static  void    Mix16Bit(sWORD *Buffer, int Length, int UnsignedOutput)
        {
        int     i, j, k;
        int     FinalSample;

        for(i=0; i<Length; i++) {
          FinalSample=0;
          for(j=0; j<5; j++) {
                ProcessWave(j);
                FinalSample+=GetSample(j);
                }
          FinalSample<<=6;
//          if(UnsignedOutput)
//            Buffer[i]=(FinalSample ^ 32768);
//          else
            Buffer[i]=FinalSample;
          }
        }

static  void    Mix16BitStereo(sWORD *Buffer, int Length, int UnsignedOutput)
        {
        int     i, j, k;
        int     FinalSampleL, FinalSampleR;

        for(i=0; i<Length; i++) {
//          FinalSample=0;
//          for(j=0; j<5; j++) {
//                ProcessWave(j);
//                FinalSample+=GetSample(j);
//                }
        ProcessWave(0);
        ProcessWave(1);
        ProcessWave(2);
        ProcessWave(3);
        ProcessWave(4);

        FinalSampleL=GetSample(0)+GetSample(1);
        FinalSampleR=GetSample(2)+GetSample(3)+GetSample(4);

          FinalSampleL<<=6;
          FinalSampleR<<=6;


//          if(UnsignedOutput)
//            Buffer[i]=(FinalSample ^ 32768);
//          else
            Buffer[i*2]=FinalSampleL;
            Buffer[i*2+1]=FinalSampleR;
          }
        }

static  void    Mix16BitStereo2(sWORD *Buffer, int Length, int UnsignedOutput)
        {
        static	double	inputL,
						inputR,
						outputL,
						outputR,
						previnputL=0,
						previnputR=0,
						prevoutputL=0,
						prevoutputR=0;
		static	double	iL1=0,
						iL2=0,
						iR1=0,
						iR2=0,
						oL1=0,
						oL2=0,
						oR1=0,
						oR2=0;

		int     i, j, k;
        int     FinalSampleL, FinalSampleR;
        int     c0, c1, c2, c3, c4, cv0, cv1, cv2;

        for(i=0; i<Length; i++) {
//          FinalSample=0;
//          for(j=0; j<5; j++) {
//                ProcessWave(j);
//                FinalSample+=GetSample(j);
//                }
        ProcessWave(0);
        ProcessWave(1);
        ProcessWave(2);
        ProcessWave(3);
        ProcessWave(4);

        ProcessWave(5);
        ProcessWave(6);
        ProcessWave(7);


        c0=GetSample(0);
        c1=GetSample(1);
        c2=GetSample(2);
        c3=GetSample(3);
        c4=GetSample(4);

        cv0=GetSample(5);
        cv1=GetSample(6);
        cv2=(GetSample(7)<<2);

        FinalSampleL=((c0+c2+c3+c4+cv0+cv2)<<6)+((c1+cv1)<<4)>>1;
        FinalSampleR=((c1+c2+c3+c4+cv1+cv2)<<6)+((c0+cv0)<<4)>>1;

		inputL=FinalSampleL;
		//outputL=FilterA*inputL+FilterA*previnputL+FilterB*prevoutputL;
		outputL = b0*inputL + b1*iL1 + b0*iL2 - a1*oL1 - a2*oL2;

		//previnputL=inputL;
		//prevoutputL=outputL;
		FinalSampleL=outputL;

		iL2=iL1;
		iL1=inputL;
		oL2=oL1;
		oL1=outputL;

		inputR=FinalSampleR;
		//outputR=FilterA*inputR+FilterA*previnputR+FilterB*prevoutputR;
		outputR = b0*inputR + b1*iR1 + b0*iR2 - a1*oR1 - a2*oR2;

		//previnputR=inputR;
		//prevoutputR=outputR;
		FinalSampleR=outputR;

		iR2=iR1;
		iR1=inputR;
		oR2=oR1;
		oR1=outputR;

		//FinalSampleL+=DCoffs;
		//FinalSampleR+=DCoffs;

		if(FinalSampleL>=32767)
			FinalSampleL=32767;
		if(FinalSampleL<=(-32768))
			FinalSampleL=-32768;
		if(FinalSampleR>=32767)
			FinalSampleR=32767;
		if(FinalSampleR<=(-32768))
			FinalSampleR=-32768;

            Buffer[i*2]=FinalSampleL<<1;
            Buffer[i*2+1]=FinalSampleR<<1;
          }
        }


// 0156-14055 (Tronds land)

#define Pi  3.141592653589793
#define Pi2 6.283185307179586

void    Mix(void *Buffer, int Length, int Is16Bit, int StereoMode)
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

		// CutOffFreq=2000.0;
        {
        int     TempBitAddr=DPCM_Wave.BitAddr,
                TempVolume=DPCM_Wave.Volume,
                TempDMAon=DPCM_Wave.DMAon;
        double  TempCycle=Cycle[DPCM_CHN];

        //FilterB=atan(3.141592653589793*CutOffFreq/(double)MixingRate);
		//FilterB=-(FilterB-1.0)/(1.0+FilterB);
		//FilterA=0.5*(1.0-FilterB);


		// Gain = pow( 10.0, Gain / 20.0 );

		double	Gain=1.0,
				Q=0.1;

		double	omega = ( Pi2 * CutOffFreq ) / (double)MixingRate,
				sn = sin( omega ),
				cs = cos( omega ),
				alpha = sn / ( 2.0 * Q );
		a0 = 1.0 / ( 1.0 + alpha );
		a1 = ( -2.0 * cs ) * a0;
		a2 = ( 1.0 - alpha ) * a0;
		b1 = ( 1.0 - cs ) * a0 * Gain;
		b0 = b1 * 0.5;


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
                    if(!StereoMode)
                      Mix16Bit(&Buffer16[SamplesMixedSoFar], MixInThisCall, 1);
                    else if(StereoMode==1)
                      Mix16BitStereo(&Buffer16[SamplesMixedSoFar<<1], MixInThisCall, 1);
                    else if(StereoMode==2)
                      Mix16BitStereo2(&Buffer16[SamplesMixedSoFar<<1], MixInThisCall, 1);

                    #endif
                    #ifdef WAP
                    if(!StereoMode)
                      Mix16Bit(&Buffer16[SamplesMixedSoFar], MixInThisCall, 0);
                    else if(StereoMode==1)
                      Mix16BitStereo(&Buffer16[SamplesMixedSoFar<<1], MixInThisCall, 0);
                    else if(StereoMode==2)
                      Mix16BitStereo2(&Buffer16[SamplesMixedSoFar<<1], MixInThisCall, 0);
                    #endif
                else
                    #ifndef WAP
                    Mix8Bit(&Buffer8[SamplesMixedSoFar], MixInThisCall, 1);
                    #endif
                    #ifdef WAP
                    Mix8Bit(&Buffer8[SamplesMixedSoFar], MixInThisCall, 0);
                    #endif

					{
					sWORD	TempL[576],
							TempR[576];
					int i,j;
					double  ResultL=0.0,
							ResultR=0.0;
					SynthChannel	ChnBackup[4];
					DPCM_Channel	DPCMBackup;
                    double          CycleBackup[5];
					memcpy(ChnBackup,SynthWave,sizeof(ChnBackup));
					memcpy(&DPCMBackup,&DPCM_Wave,sizeof(DPCMBackup));
					memcpy(CycleBackup,Cycle,sizeof(CycleBackup));

					// Mix16BitStereo2(&Buffer16[(SamplesMixedSoFar+MixInThisCall+32)<<1], 32, 0);

					//if(MixInThisCall>32)
					//for(j=0; j<(MixInThisCall-32); j++) {
					for(j=0; j<MixInThisCall; j++) {
					ResultL=0.0;
					ResultR=0.0;
					for(i=0; i<32; i++)
					  //ResultL+=((double)Buffer16[((SamplesMixedSoFar+j+i)<<1)]*1.0);
                      ResultL+=((double)Buffer16[((SamplesMixedSoFar+j+i)<<1)]*OldFilter[i]);
					for(i=0; i<32; i++)
					  //ResultR+=((double)Buffer16[((SamplesMixedSoFar+j+i)<<1)+1]*1.0);
                      ResultR+=((double)Buffer16[((SamplesMixedSoFar+j+i)<<1)+1]*OldFilter[i]);

					//Buffer16[((SamplesMixedSoFar+j)<<1)]=ResultL;
					//Buffer16[((SamplesMixedSoFar+j)<<1)+1]=ResultR;
					//ResultL=Buffer16[((SamplesMixedSoFar+j)<<1)]*1.25;
					//ResultR=Buffer16[((SamplesMixedSoFar+j)<<1)+1]*1.25;

					if(ResultL<(-32767)) ResultL=-32767;
					if(ResultL>32767) ResultL=32767;
					if(ResultR<(-32767)) ResultR=-32767;
					if(ResultR>32767) ResultR=32767;
					TempL[j]=ResultL;
					TempR[j]=ResultR;
                    }
					for(j=0; j<MixInThisCall; j++) {
					//Buffer16[((SamplesMixedSoFar+j)<<1)]=TempL[j];
					//Buffer16[((SamplesMixedSoFar+j)<<1)+1]=TempR[j];
					}


					/*previnputL=prevoutputL=previnputR=prevoutputR;

					for(i=0; i<MixInThisCall; i++) {
						inputL=Buffer16[(SamplesMixedSoFar+i)<<1];
						outputL=FilterA*inputL-FilterA*previnputL+FilterB*prevoutputL;
						previnputL=inputL;
						prevoutputL=outputL;
						Buffer16[(SamplesMixedSoFar+i)<<1]=outputL;

						inputR=Buffer16[((SamplesMixedSoFar+i)<<1)+1];
						outputR=FilterA*inputR-FilterA*previnputR+FilterB*prevoutputR;
						previnputR=inputR;
						prevoutputR=outputR;
						Buffer16[((SamplesMixedSoFar+i)<<1)+1]=outputR;
						}*/

					// output = a*input - a*previnput + b*prevoutput;
					// previnput = input; prevoutput = output;

					memcpy(SynthWave,ChnBackup,sizeof(ChnBackup));
					memcpy(&DPCM_Wave,&DPCMBackup,sizeof(DPCMBackup));
					memcpy(Cycle,CycleBackup,sizeof(CycleBackup));
					}

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


			if(CallInsideMixer) CallInsideMixer();
			NumFramesPlayed++;
            SamplesTilNextPlayerCall=MixingRate/PlayBackRate;}

            }
        }

