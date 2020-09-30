#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <mem.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <graph.h>

#include "typedefs.h"
#include "dephs.h"
#include "file.h"
#include "scrtxt.h"
#include "nt2.h"
#include "system.h"
#include "keyb.h"

//#include "mainpic.h"
#include "nt2_lo2.h"
#include "nt2_smp.c"
#include "dpcm_win.h"
#include "greets.h"
#include "neshead.h"
#include "porno.h"
#include "endpic2.h"


#include "gus\nesess.h"

#define DESC_X          30
#define DESC_Y          2

struct  ned_header_struc ned_header;

struct  inst_struc      inst[MAX_INSTRUMENTS];

InstDPCM        SampleInst[MAX_INSTRUMENTS_DPCM];

ChannelPattern  *ChnPattern[MAX_CHANNELS]={NULL, NULL, NULL, NULL, NULL};
//PatternDPCM     *SamplePattern=NULL;

char    InstName[MAX_INSTRUMENTS_DPCM][DPCM_INAME_LENGTH];
char    SampleName[MAX_INSTRUMENTS_DPCM][MAX_DPCM_SAMPLES][DPCM_SNAME_LENGTH];

BYTE    NoteTable[MAX_INSTRUMENTS_DPCM][MAX_DPCM_OCTAVES][12];

float   note_phreq[8][13];

/* int     Note2PeriodTab[96]={ 1614, 1523, 1438, 1357, 1281, 1209, 1141, 1077, 1016, 959, 905, 855,
                                807, 761, 719, 678, 640, 604, 570, 538, 508, 479, 452, 427,
                                403, 380, 359, 339, 320, 302, 285, 269, 254, 239, 226, 213, 
                                201, 190, 179, 169, 160, 151, 142, 134, 127, 119, 113, 106, 
                                100, 95, 89, 84, 80, 75, 71, 67, 63, 59, 56, 53,
                                50, 47, 44, 42, 40, 37, 35, 33, 31, 29, 28, 26, 
                                25, 23, 22, 21, 20, 18, 17, 16, 15, 14, 14, 13, 
                                12, 11, 11, 10, 10, 9, 8, 8, 7, 7, 7, 6};*/
                            
/*int     Note2PeriodTab[108]={ 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2034,1920,1812,
                                1710, 1614, 1524, 1438, 1358, 1281, 1209, 1142, 1077, 1017, 960, 906,
                                855,  807,  762,  719,  679,  641,  605,  571,  539,  509,  480, 453,
                                428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240, 227,
                                214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120, 113,
                                107,  101,  95,   90,   85,   80,   76,   71,   67,   64,   60,  57,
                                53,   50,   48,   45,   42,   40,   38,   36,   34,   32,   30,  28,
                                27,   25,   24,   22,   21,   20,   19,   18,   17,   16,   15,  14,
                                13,   13,   12,   11,   11,   10,   9,    9,    8,    8,    7,   7};*/

int     Note2PeriodTab[108]={   3420, 3228, 3048, 2876, 2716, 2607, 2418, 2284, 2047, 2034,1920,1812,
                                1710, 1614, 1524, 1438, 1358, 1281, 1209, 1142, 1077, 1017, 960, 906,
                                855,  807,  762,  719,  679,  641,  605,  571,  539,  509,  480, 453,
                                428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240, 227,
                                214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120, 113,
                                107,  101,  95,   90,   85,   80,   76,   71,   67,   64,   60,  57,
                                53,   50,   48,   45,   42,   40,   38,   36,   34,   32,   30,  28,
                                27,   25,   24,   22,   21,   20,   19,   18,   17,   16,   15,  14,
                                13,   13,   12,   11,   11,   10,   9,    9,    8,    8,    7,   7};

sBYTE   LCounterTable[32]={5, 127, 10, 1, 20, 2, 40, 3,
                           80, 4, 30, 5, 7, 6, 13, 7,
                           6, 8, 12, 9, 24, 10, 48, 11,
                           96, 12, 36, 13, 8, 14, 16, 15};


char    lnote[13]={0, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3};

/* char    *note_char[14]={"--", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-", "C-", "OF"};*/
char    *note_char[14]={"--", "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-", "OF"};

char    BuggyNote2Normal[12]={1,2,3,4,5,6,7,8,9,10,11,0};

char    real_colpos[NUMBER_OPH_COLS]={0, 2, 4, 5, 7, 8, 9};

#define NUMBER_OPH_WARNS        9

char    *exit_warns[NUMBER_OPH_WARNS]= {
        "Noo! DON'T EXIT! I promise the next version will be better!",
        "Hello, nerd!",
        "Don't go! The NES-scene needs you! Err.. it does exist? Right?..",
        "The Real NERDS 'R' US rule? [y/n]",
        "REALLY exit NT2? [(N)o/(N)o/(M)aybe later/(O)ops]",
        "This program has caused an illegal operation and will be terminated? [y/n]",
        "The document could not be registered. Perhaps it is already open? [y/n]",
        "Out of memory. Please close some applications before trying again.",
        "How do we print a bluescreen???"};


BYTE    Reg4015=0;
long    curr_orderpos=0;
long    curr_row=0;
long    curr_chn=0;
long    curr_chncol=0;
long    curr_pat=0;
long    curr_octave=0;
long    curr_inst=1;
int     CurrSampleInst=1;
int     CurrSample=0;
long    edit_state=EDIT_PAT;
int     Editor=PATTERN_EDITOR;
int     SampleEditorState=EDIT_SAMPLE_NAMES;
long    curr_speed=6;
long    order_tot=1;
int     curr_instrow=0;
int     curr_order=0;
int     RestartPosition=0;

int     CurrOrderChn=0;
int     CurrSampleSetting=0;
int     InameWindowLine=0;
int     FirstIname=0;

int     CurrNoteTableNote=0;
int     CurrNoteTableOctave=0;
int     NoteTableSmpOrFreq=0;

long    nt_sound_delay=0;
unsigned short  kb_shipht_state;


boolean done=0;

char    key_pressed;


const int SineWave[32]={0,   24, 49, 74, 97,120,141,161,
                        180,197,212,224,235,244,250,253,
                        255,253,250,244,235,224,212,197,
                        180,161,141,120, 97, 74, 49, 24};


char    hex_tab[17]="0123456789ABCDEF";
char    *boo_tab[2]={"OFF", "ON!"};


void    nt_UpdatePosInstNames(void);
struct  ned_inst_struc ned_inst[31];

signed char    order_list[128];
BYTE    OrderList[NT_MAX_ORDER][MAX_CHANNELS];

unsigned short pat_ophph[MAX_PATTERNS];

char    *ned_phile=NULL;


char    ned_philename[20];

char    eph2nedeph[16]={0, 4, 5, 6, 0, 0, 0, 7, 0, 0, 0, 0, 1, 3, 0, 2};
char    nedeph2eph[8]={0, 0xC, 15, 0xD, 0x1, 0x2, 0x3, 7};


int     nt_misc_init(void);
unsigned char   get_key(void);
void    nt_update_pos(void);
void    nt_draw_misc(void);
void    draw_insted(void);
void    draw_order(void);
void    DrawPatternEditor(void);
void    write_54chars(long x, long y, char the_char, char color, char bcolor);
void    write_upperbord(int temp_patrow_scr);
void    write_lowerbord(int temp_patrow_scr);
void    DrawSampleEditor(void);


void    nt_print_alert(char *string);
void    nt_update_pos(void);
void    nt_update_pos_pat(void);
void    nt_update_pos_inst(void);
void    nt_UpdatePosSampleEditor(void);
void    nt_UpdatePosSampleSettings(void);
void    nt_UpdatePosSampleNames(void);
void    nt_UpdatePosNoteTable(void);

void    nt_write_note(int note, int add_octave);

void    PlayNote(int Chn, int Inst, int Note, int Oct);


void    CopyOrderEntry(int Src, int Dest);
void    ClearOrderEntry(int entry);
void    ClearOrder2Max(int StartEntry);
void    CopyPattern(ChannelPattern *Src, ChannelPattern *Dest);
void    ClearPattern(ChannelPattern *Pat);
int     PatternIsEmpty(ChannelPattern *Pat);
void    ResetEditors(void);


int     SaveNedInfo20(char *FileName, DWORD *FileOffset, InfoHeader *Info);
int     SaveNedData20(char *FileName, DWORD FileOffset, InfoHeader *Info);
int     SaveNed20(char *FileName);

int     LoadNed(char *FileName);
int     LoadNed20(char *FileName);
int     LoadNed10(char *FileName);
void    ConvertNed10Pat2ChnPat(BYTE *Ned10Pat, ChannelPattern *P_A, ChannelPattern *P_B, ChannelPattern *C, ChannelPattern *P_D);


void    GetSnoteString(struct snote_struc *TheSnote, char *SnoteString, BYTE *SnoteColor, int Lit);

BYTE    *Layout=NT2_LO2;
int     WhichPiccy=0;



void    DrawFgColLine_H(int StartX, int EndX, int Y, BYTE Color, BYTE *Buffer);
void    DrawFgColLine_V(int X, int StartY, int EndY, BYTE Color, BYTE *Buffer);

void    DrawBgColLine_H(int StartX, int EndX, int Y, BYTE Color, BYTE *Buffer);
void    DrawBgColLine_V(int X, int StartY, int EndY, BYTE Color, BYTE *Buffer);

void    DrawBothColLine_H(int StartX, int EndX, int Y, BYTE Color, BYTE *Buffer);

void    Blit(int x, int y, int Width, int Height, BYTE *Src, BYTE *Dest);


void    Word2HexStr(int Val, char *Str);
void    PrintStringBoth(int x, int y, char *Str, BYTE Color, void *Dest);

void    My_gets(char *String);

void    My_gets(char *String)
{
int     EnterPressed=0, i=0, c;

while(!EnterPressed)
  {
  if(kbhit()) {
    c=getche();
    if(c==13) {
      EnterPressed=1;
      String[i]=0;
      }
    else {
      String[i]=c;
      i++;
      }
    }
  }
}
    

void    SelectFile(char *FileName, char *FileMask);

void    SelectFile(char *FileName, char *FileMask)
{
My_gets(FileName);
}

int     scan2hexdig(char   scan_code, char limit);

#define PERIOD_LIMIT    10

typedef struct {
int     Period;
int     LastTone;
int     LastInst;
int     Tone;
int     Inst;
int     Effect;              
int     EffParm;
int     PortaTone;
int     PortaSpeed;
int     AutoPorta;
// int     VolumeFlag;
// int     PeriodFlag;
int     Reg0;
int     Reg1;
int     Reg3;
int     Volume;
int     VolumeSlide;
int     AutoVolumeSlide;
int     Arpeggio;
int     AutoArpX;
int     AutoArpY;
int     AutoArpZ;
int     VibratoPos;
int     AutoVibratoSpeed;
int     AutoVibratoDepth;
int     VibratoSpeed;
int     VibratoDepth;
int     TremoloPos;
int     AutoTremoloSpeed;
int     AutoTremoloDepth;
int     TremoloSpeed;
int     TremoloDepth;
int     LoopedNoise;
int     VblanksLeft;
int     BigTick;
int     ReversedArpeggio;
int     NonLoopedArpeggio;
} SChn;

SChn    Chn[MAX_CHANNELS];


int     CurrTick=0;
//long    BigTick=0;
int     IsPlaying=0;
int     EmulateNTSC=0;
int     PalCounter=1;


void    ResetPlayer(void)
        {
        memset(&Chn[0], 0, sizeof(SChn)*MAX_CHANNELS);
        }

void    GetSNote(int C)
        {
        int     Pat;
        struct snote_struc *SNote;
        Pat=OrderList[curr_order][C];
        SNote=&(ChnPattern[C][Pat].Row[curr_row]);

        if((*SNote).note==NOTE_OPHPH)
          Chn[C].Tone=97;
        else
          Chn[C].Tone=((*SNote).octave<<3)+((*SNote).octave<<2)+(*SNote).note;
        if((C==NOSWAV_CHN) && Chn[C].Tone)
          Chn[C].Tone=((15-(Chn[C].Tone-1)) & 0xF)+1;
        Chn[C].Inst=((*SNote).inst_num1<<4)+(*SNote).inst_num2;
        Chn[C].Effect=(*SNote).ephphect;
        Chn[C].EffParm=((*SNote).e_parm1<<4)+(*SNote).e_parm2;
        }



#define tEFF_PORTAUP    0x1
#define tEFF_PORTADOWN  0x2
#define tEFF_TONEPORTA  0x3
#define tEFF_VIBRATO    0x4
#define tEFF_TREMOLO    0x7
#define tEFF_ARPEGGIO   0x8
#define tEFF_VOLSLIDE   0xA
#define tEFF_PATTERNJMP 0xB
#define tEFF_MODVOLUME  0xC
#define tEFF_PATTERNBRK 0xD
#define tEFF_SETSPEED   0xF



void    SetVolume(int C, int Volume)
        {
        if(C!=TRIWAV_CHN) {
          if(Volume<0) Volume=0;
          if(Volume>63) Volume=63;
          Write2SoundReg(0x4000+C*4, (Chn[C].Reg0 | (Volume>>2)));
          }
        }

void    SetPeriod(int C, int Period, int TimeLength)
        {
        if(C==NOSWAV_CHN) {
          Write2SoundReg(0x400E, (Period & 0xF)+(Chn[C].LoopedNoise<<7));
          Write2SoundReg(0x400F, 8);
          }
        else {
          if(Period<0)
            Period=0;
          if(Period>=2048)
            Period=2047;
          Write2SoundReg(0x4002+C*4, (Period & 0xFF));
          Write2SoundReg(0x4003+C*4, (Period>>8) | 8);
          }
        }


void    DoVibrato(int C, int Speed, int Depth)
        {
        int     SinePos;
        int     Delta;
        int     Period;

        if(Chn[C].VibratoPos>=0)
          SinePos=Chn[C].VibratoPos & 31;
        else
          SinePos=(~Chn[C].VibratoPos) & 31;
        Delta=(SineWave[SinePos]*Depth)>>7;

        if(Chn[C].VibratoPos>=0)
          Period=Chn[C].Period+Delta;
        else
          Period=Chn[C].Period-Delta;

        SetPeriod(C, Period, 8);
        // Chn[C].PeriodFlag=0;

        Chn[C].VibratoPos+=Speed;
        if(Chn[C].VibratoPos>=32) Chn[C].VibratoPos-=64;
        }


void    DoTremolo(int C, int Speed, int Depth)
        {
        int     SinePos;
        int     Delta;
        int     Volume;

        if(Chn[C].TremoloPos>=0)
          SinePos=Chn[C].TremoloPos & 31;
        else
          SinePos=(~Chn[C].TremoloPos) & 31;
        Delta=(SineWave[SinePos]*Depth)>>6;

        if(Chn[C].TremoloPos>=0)
          Volume=Chn[C].Volume+Delta;
        else
          Volume=Chn[C].Volume-Delta;

        SetVolume(C, Volume);

        Chn[C].TremoloPos+=Speed;
        if(Chn[C].TremoloPos>=32) Chn[C].TremoloPos-=64;
        }


void    DoEffsT0(int C)
        {
        // Portamento
        if((Chn[C].Effect==tEFF_PORTAUP) || (Chn[C].Effect==tEFF_PORTADOWN)) {
          if(Chn[C].EffParm)
            Chn[C].PortaSpeed=Chn[C].EffParm;
          }
        // Tone Portamento
        if(Chn[C].Effect==tEFF_TONEPORTA) {
          if(Chn[C].EffParm)
            Chn[C].PortaSpeed=Chn[C].EffParm;
          if(Chn[C].Tone)
            Chn[C].PortaTone=Chn[C].Tone;
          }
        // Vibrato
        if(Chn[C].Effect==tEFF_VIBRATO)
          if(Chn[C].EffParm) {
            Chn[C].VibratoSpeed=(Chn[C].EffParm>>4) & 0xF;
            Chn[C].VibratoDepth=Chn[C].EffParm & 0xF;
            }
          


        // Set Volume
        if(Chn[C].Effect==tEFF_MODVOLUME) {
            Chn[C].Volume=Chn[C].EffParm & 0x3F;
          }
        // Volume Slide
        if(Chn[C].Effect==tEFF_VOLSLIDE)
          if(Chn[C].EffParm) {
            Chn[C].VolumeSlide=Chn[C].EffParm;
            }
        // Tremolo
        if(Chn[C].Effect==tEFF_TREMOLO)
          if(Chn[C].EffParm) {
            Chn[C].TremoloSpeed=(Chn[C].EffParm>>4) & 0xF;
            Chn[C].TremoloDepth=Chn[C].EffParm & 0xF;
            }
        // Arpeggio
        if(Chn[C].Effect==tEFF_ARPEGGIO) {
          if(Chn[C].EffParm)
            Chn[C].Arpeggio=Chn[C].EffParm;
          //Chn[C].PeriodFlag=1;
          Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone-1)%96];
          SetPeriod(C, Chn[C].Period, 8);
          goto SkipPeriodSet;
          }
        else {
          if(Chn[C].LastInst) { // Do AutoArpeggio
            if((Chn[C].BigTick<4) || (!Chn[C].NonLoopedArpeggio)) {
                int   ArpX=Chn[C].AutoArpX,
                      ArpY=Chn[C].AutoArpY,
                      ArpZ=Chn[C].AutoArpZ;

                if(ArpX || ArpY || ArpZ) {
                        //Chn[C].PeriodFlag=1;
                        if((Chn[C].BigTick & 3)==0)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone-1)%96];
                        if((Chn[C].BigTick & 3)==1)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ArpX-1)%96];
                        if((Chn[C].BigTick & 3)==2)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ArpY-1)%96];
                        if((Chn[C].BigTick & 3)==3)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ArpZ-1)%96];
                        SetPeriod(C, Chn[C].Period, 8);
                        goto SkipPeriodSet;

                        }
                
                }
              }
          }


if((Chn[C].Tone) && (Chn[C].Effect!=tEFF_TONEPORTA))
        SetPeriod(C, Chn[C].Period, 8);
SkipPeriodSet:

        // Set speed
        if(Chn[C].Effect==tEFF_SETSPEED)
          curr_speed=Chn[C].EffParm;

        // Pattern Break
        if(Chn[C].Effect==tEFF_PATTERNBRK) {
          curr_row=(Chn[C].EffParm-1);
          if((++curr_order)>=order_tot)
            curr_order=RestartPosition;
          }

        }
         
void    DoEffsTx(int C)
        {


          if(!(Chn[C].VblanksLeft & 0x80)) {
            if((--Chn[C].VblanksLeft)<0) {
              Chn[C].VblanksLeft=0;
              Reg4015&=(~(1<<C));
              }
            }


        // Volume Slide (has priority over Auto Volume Slide)
        if((Chn[C].Effect==tEFF_VOLSLIDE)) {
            Chn[C].Volume+=(Chn[C].VolumeSlide>>4) & 0xF;
            Chn[C].Volume-=Chn[C].VolumeSlide & 0xF;
          }
        // AutoVolumeSlide
        else {
          if(Chn[C].AutoVolumeSlide!=0) {
            Chn[C].Volume+=Chn[C].AutoVolumeSlide;
            }
          }
        // Tremolo (has priority over AutoTremolo)
        if(Chn[C].Effect==tEFF_TREMOLO) {
          DoTremolo(C, Chn[C].TremoloSpeed, Chn[C].TremoloDepth);
          goto SkipVolumeSet;
          }
        // AutoTremolo
        else {
          if((Chn[C].AutoTremoloSpeed) || (Chn[C].AutoTremoloDepth)) {
            DoTremolo(C, Chn[C].AutoTremoloSpeed, Chn[C].AutoTremoloDepth);
            goto SkipVolumeSet;
            }
          }


        if(Chn[C].Volume<0) Chn[C].Volume=0;
        if(Chn[C].Volume>63) Chn[C].Volume=63;

          SetVolume(C, Chn[C].Volume);


SkipVolumeSet:

        // Arpeggio (has priority over AutoArpeggio)
        if(Chn[C].Effect==tEFF_ARPEGGIO) {
          int   ParmX=(Chn[C].Arpeggio>>4) & 0xF;
          int   ParmY=Chn[C].Arpeggio & 0xF;
          // Chn[C].PeriodFlag=1;

          if((CurrTick%3)==0)
                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone-1)%96];
          if((CurrTick%3)==1)
                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ParmX-1)%96];
          if((CurrTick%3)==2)
                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ParmY-1)%96];
          }
        else {
          if(Chn[C].LastInst) { // Do AutoArpeggio
            if((Chn[C].BigTick<4) || (!Chn[C].NonLoopedArpeggio)) {
                int   ArpX=Chn[C].AutoArpX,
                      ArpY=Chn[C].AutoArpY,
                      ArpZ=Chn[C].AutoArpZ;

                if(ArpX || ArpY || ArpZ) {
                        // Chn[C].PeriodFlag=1;
                        if((Chn[C].BigTick & 3)==0)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone-1)%96];
                        if((Chn[C].BigTick & 3)==1)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ArpX-1)%96];
                        if((Chn[C].BigTick & 3)==2)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ArpY-1)%96];
                        if((Chn[C].BigTick & 3)==3)
                                Chn[C].Period=Note2PeriodTab[(Chn[C].LastTone+ArpZ-1)%96];
                        }
              }
                }
          }
        // Portamento Up
        if(Chn[C].Effect==tEFF_PORTAUP) {
          // Chn[C].PeriodFlag=1;
          Chn[C].Period-=Chn[C].PortaSpeed;
          goto PortaDone;
          }
        // Portamento Down
        if(Chn[C].Effect==tEFF_PORTADOWN) {
          // Chn[C].PeriodFlag=1;
          Chn[C].Period+=Chn[C].PortaSpeed;
          goto PortaDone;
          }
        // Tone Portamento
        if(Chn[C].Effect==tEFF_TONEPORTA) {
          int   PTonePeriod=Note2PeriodTab[Chn[C].PortaTone-1];

          if(Chn[C].Period<PTonePeriod) {
            Chn[C].Period+=Chn[C].PortaSpeed;
            if(Chn[C].Period>PTonePeriod)
              Chn[C].Period=PTonePeriod;
            // Chn[C].PeriodFlag=1;
            }
          if(Chn[C].Period>PTonePeriod) {
            Chn[C].Period-=Chn[C].PortaSpeed;
            if(Chn[C].Period<PTonePeriod)
              Chn[C].Period=PTonePeriod;
            // Chn[C].PeriodFlag=1;
            }
          goto PortaDone;
          }

        // Auto Portamento
        if(Chn[C].AutoPorta!=0) {
          // Chn[C].PeriodFlag=1;
          Chn[C].Period+=Chn[C].AutoPorta;
          }

PortaDone:

        // Vibrato (has priority over AutoVibrato)
        if(Chn[C].Effect==tEFF_VIBRATO) {
          DoVibrato(C, Chn[C].VibratoSpeed, Chn[C].VibratoDepth);
          goto SkipPeriodSet;
          }
        // Autovibrato
        else {
          if((Chn[C].AutoVibratoSpeed) || (Chn[C].AutoVibratoDepth)) {
            DoVibrato(C, Chn[C].AutoVibratoSpeed, Chn[C].AutoVibratoDepth);
            goto SkipPeriodSet;
            }
          }

          //if(C==NOSWAV_CHN)
          //  SetPeriod(C, Chn[C].Period, 8);
          //else
          //  if(Chn[C].PeriodFlag)
              SetPeriod(C, Chn[C].Period, 8);

SkipPeriodSet:

          if(Chn[C].VblanksLeft!=0) {
            if(C==TRIWAV_CHN)
              Write2SoundReg(0x4008, 0xFF);
            }
        }


void    UpdateT0(void)
        {
        int i, j, k;

        for(i=0; i<MAX_CHANNELS; i++)
          GetSNote(i);

        for(i=0; i<MAX_A_CHANNELS; i++) {

          if(Chn[i].Inst) {
            Chn[i].LastInst=Chn[i].Inst;

            if(inst[Chn[i].LastInst-1].hold_note)
              Chn[i].VblanksLeft=0x80;
            else
              Chn[i].VblanksLeft=inst[Chn[i].LastInst-1].active_tlength;

            Chn[i].Reg0=(inst[Chn[i].LastInst-1].duty_cycle<<6)+(inst[Chn[i].LastInst-1].hold_note<<5)+
                        (inst[Chn[i].LastInst-1].envelope_phixed<<4);

            Reg4015|=(1<<i);

            Chn[i].Volume=(inst[Chn[i].LastInst-1].volume)<<2;
            Chn[i].AutoVolumeSlide=(-inst[Chn[i].LastInst-1].VolumeSlide);
            if(inst[Chn[i].LastInst-1].VolumeSlideDir)
              Chn[i].AutoVolumeSlide=(-Chn[i].AutoVolumeSlide);

            

            Chn[i].NonLoopedArpeggio=inst[Chn[i].LastInst-1].NonLoopedArpeggio;
            if(Chn[i].ReversedArpeggio=inst[Chn[i].LastInst-1].ReversedArpeggio) {
              Chn[i].AutoArpX=-inst[Chn[i].LastInst-1].ArpX;
              Chn[i].AutoArpY=-inst[Chn[i].LastInst-1].ArpY;
              Chn[i].AutoArpZ=-inst[Chn[i].LastInst-1].ArpZ;
              }
            else {  
              Chn[i].AutoArpX=inst[Chn[i].LastInst-1].ArpX;
              Chn[i].AutoArpY=inst[Chn[i].LastInst-1].ArpY;
              Chn[i].AutoArpZ=inst[Chn[i].LastInst-1].ArpZ;
              }
            if(inst[Chn[i].LastInst-1].PortaDir)
              Chn[i].AutoPorta=-inst[Chn[i].LastInst-1].Porta;
            else
              Chn[i].AutoPorta=inst[Chn[i].LastInst-1].Porta;

            Chn[i].AutoTremoloSpeed=inst[Chn[i].LastInst-1].TremoloSpeed;
            Chn[i].AutoTremoloDepth=inst[Chn[i].LastInst-1].TremoloDepth;

            Chn[i].AutoVibratoSpeed=inst[Chn[i].LastInst-1].VibratoSpeed;
            Chn[i].AutoVibratoDepth=inst[Chn[i].LastInst-1].VibratoDepth;

            Chn[i].LoopedNoise=inst[Chn[i].LastInst-1].LoopedNoise;

            if(inst[Chn[i].LastInst-1].hold_note)
              Chn[i].VblanksLeft=0x80;
            else
              Chn[i].VblanksLeft=inst[Chn[i].LastInst-1].active_tlength;
            }

          if(Chn[i].Tone==97)
            Reg4015&=(~(1<<i));
            //Chn[i].VblanksLeft=0;


          if((Chn[i].Tone) && (Chn[i].Tone!=97)) {
            Chn[i].BigTick=0;
            if(inst[Chn[i].LastInst-1].hold_note)
              Chn[i].VblanksLeft=0x80;
            else
              Chn[i].VblanksLeft=inst[Chn[i].LastInst-1].active_tlength;



            Chn[i].LastTone=Chn[i].Tone;
            Reg4015|=(1<<i);

            Chn[i].TremoloPos=0;

            if(Chn[i].Effect!=tEFF_TONEPORTA) {

              Chn[i].VibratoPos=0;
              if(i==NOSWAV_CHN) {
                Chn[i].Period=Chn[i].LastTone-1;
                  //Write2SoundReg(0x400F, 8);
                }
              else
                Chn[i].Period=Note2PeriodTab[Chn[i].Tone-1];
              //Chn[i].PeriodFlag=1;
              }
            }

 
          DoEffsT0(i);
          }
        //GetSNote(DPCM_CHN);

          if(Chn[DPCM_CHN].Tone || Chn[DPCM_CHN].Inst) // SHIT...
          {
          int Oct=(Chn[DPCM_CHN].Tone-1)/12,
              Note=((int)((Chn[DPCM_CHN].Tone-1)-Oct*12))+1;
          PlayNote(DPCM_CHN, Chn[DPCM_CHN].Inst, Note, Oct);
          }
        }



void    PlayNED(void)
        {
        int i, j, k;


        if(!IsPlaying) return;

        if(EmulateNTSC) 
        if((--PalCounter)<=0) {
          PalCounter=6;
          return;
          }

        if(curr_speed) {

        // Reset update flags
        for(i=0; i<MAX_A_CHANNELS; i++)
          //Chn[i].PeriodFlag=0;


          if((++CurrTick)>=curr_speed) {
            CurrTick=0;
            if(curr_row>=64) {
              curr_row=0;
              if((++curr_order)>=order_tot)
                        curr_order=RestartPosition;
              }
            UpdateT0();
            curr_row++;

        for(i=0; i<MAX_A_CHANNELS; i++) {

          if(!(Chn[i].VblanksLeft & 0x80)) {
            if((--Chn[i].VblanksLeft)<0) {
              Reg4015&=(~(1<<i));
              Chn[i].VblanksLeft=0;
              }
            }


          if(Chn[i].VblanksLeft!=0) {
            if(i==TRIWAV_CHN)
              Write2SoundReg(0x4008, 0xFF);
            }

              if((Chn[i].Effect==tEFF_MODVOLUME) || (Chn[i].Inst)) {
                if(Chn[i].Volume<0) Chn[i].Volume=0;
                if(Chn[i].Volume>63) Chn[i].Volume=63;

                SetVolume(i, Chn[i].Volume);
                }

        //if(i==NOSWAV_CHN)
        //  SetPeriod(i, Chn[i].Period, 8);
        //else
          //if(Chn[i].PeriodFlag)
            //SetPeriod(i, Chn[i].Period, 8);

          }
            }
          else {
            for(i=0; i<MAX_A_CHANNELS; i++) {
              DoEffsTx(i);
              }
            }
          {
          if(!(ReadSoundReg(0x4015) & 0x10))
            Reg4015&=0xF;
          Write2SoundReg(0x4015, Reg4015);
          }

        }  // end if(curr_speed)
        Chn[0].BigTick++;
        Chn[1].BigTick++;
        Chn[2].BigTick++;
        Chn[3].BigTick++;
        }



void    main(void)
        {
        int     RshiftReleased, LshiftReleased;
        int     i, j;
        _setcolor(15);
        if(!(scrtxt_vidbuphph=(unsigned char *)malloc(16000))) {
                printf("REALLY out of memory!");
                exit(0);
                }
        scrtxt_vidmem=scrtxt_vidbuphph;


        if(!nt_misc_init())
                {
                printf("Initialization error\n");
                return;
                }
        
        CallInsideMixer=PlayNED;

        scrtxt_set_lores();
        scrtxt_set_hires();
        //scrtxt_set_blink(0);        

        system("NT2FNT");

//        getch();

        if(EmulateNTSC) {
          VBlankSpeed=VBLANK_SPEED_NTSC;
          PlayBackRate=VBLANK_SPEED_NTSC;
          }
        else {
          VBlankSpeed=VBLANK_SPEED_PAL;
          PlayBackRate=VBLANK_SPEED_PAL;
          }

        if(!NesessInitialize()) {
                printf("Failed to initialize sound system!\n");
                return;
                }

        while(!done)
                {
                long    *sys_clock=(long *)0x46c;
                srand(*sys_clock);
                // if((sys_phrame_counter-nt_sound_delay)>5) nosound();

                
                nt_update_pos();           // Read input & update positions

                if((sys_lepht_ctrl_pressed) && (key_pressed==KEY_L))
                  {
                  scrtxt_set_cursor_pos(56, 33);
                  My_gets(ned_philename);
                  if(!LoadNed(ned_philename))
                    {
                    printf("Apa!\n");
                    getch();
                    }
                  }
                if((sys_lepht_ctrl_pressed) && (key_pressed==KEY_S))
                  {
                  scrtxt_set_cursor_pos(56, 33);
                  My_gets(ned_philename);
                  if(!SaveNed20(ned_philename))
                    {
                    printf("Apa!\n");
                    getch();
                    }
                  }

                if(!sys_right_shipht_pressed)
                        RshiftReleased=1;
                if(!sys_lepht_shipht_pressed)
                        LshiftReleased=1;


                if(sys_right_shipht_pressed && (RshiftReleased)) {
                  int Temp=IsPlaying;
                  int i;
                  if(!Temp) {
                    curr_row=0;
                    CurrTick=curr_speed;
                    Chn[0].BigTick=0;
                    Chn[0].BigTick=0;
                    Chn[0].BigTick=0;
                    Chn[0].BigTick=0;

                    ResetPlayer();
                    Reg4015=0x0;
                    //for(i=0; i<0x14; i++)
                    //  Write2SoundReg(0x4000+i, 0x0);
                    Write2SoundReg(0x4000, 0x10);
                    Write2SoundReg(0x4004, 0x10);
                    Write2SoundReg(0x4008, 0x00);
                    Write2SoundReg(0x400C, 0x10);
                    //for(i=0; i<4; i++)
                    //  Write2SoundReg(0x4003+i*4, 0x18);


                    Write2SoundReg(0x4015, 0x0);
                    IsPlaying=1;
                    }
                  else {
                    IsPlaying=0;
                    Reg4015=0x0;
                    Write2SoundReg(0x4015, 0x0);
                    //for(i=0; i<0x14; i++)
                    //  Write2SoundReg(0x4000+i, 0x0);
                    for(i=0; i<4; i++)
                      Write2SoundReg(0x4003+i*4, 0x18);
                    }
                  RshiftReleased=0;
                  }
                    

                /* if(sys_lepht_shipht_pressed && (LshiftReleased)) {
                  int Temp=IsPlaying;
                  if(!Temp) {
                    curr_order=0;
                    curr_row=0;
                    CurrTick=curr_speed;
                    IsPlaying=1;
                    Reg4015=0xF;
                    Write2SoundReg(0x4015, 0xF);
                    }
                  else {
                    IsPlaying=0;
                    Reg4015=0x0;
                    Write2SoundReg(0x4015, 0x0);
                    }
                  LshiftReleased=0;
                  }*/

                

                if(key_pressed==KEY_ENTER)
                  {
                  InfoHeader Info;
                  remove("temp.dat");
                  
                  SaveNedInfo20("TEMP.IHD",0,&Info);

                  Info.HeaderSize=0;

                  // !!
                  Info.NumSampleInstrumentsNTSC=0;
                  Info.NumSampleInstrumentsPAL=0;
                  Info.SampleDataSizeNTSC=0;
                  Info.SampleDataSizePAL=0;


                  if(!SaveNedData20("TEMP.DAT",0,&Info))
                    {
                    printf("Apa!\n");
                    exit(0);
                    }
                  //NesessShutdown();
                  //system("user.bat>NUL");
                  //if(!NesessInitialize()) {
                  //      printf("Failed to initialize sound system!\n");
                  //      return;
                  //      }
                  
                  scrtxt_set_hires();
                  //scrtxt_set_blink(0);
                  }

                memcpy((BYTE *)scrtxt_vidbuphph, Layout, 80*50*2);
                nt_draw_misc();
                scrall_wait_vr();
                memcpy((BYTE *)0xB8000, scrtxt_vidbuphph, 80*50*2);
                }

        sys_remove_timer_isr();
        sys_set_timer_speed(SYS_TIMER_SPEED_18HZ);

        scrtxt_set_lores();
        scrtxt_set_hires();
        //scrtxt_set_blink(1);
        nosound();
                scrtxt_put_pic_lines_clear(end_pic, 50);
                scrall_wait_vr();
                scrtxt_copy2vidmem();
                scrtxt_vidmem=(unsigned char *)0xb8000;
                scrtxt_set_cursor_pos(0, 47);
        NesessShutdown();
        return;
        }


int     nt_misc_init(void)
        {
        int     i, j, k;
        float   temphreq;
        key_pressed=0;
        curr_orderpos=0;
        curr_row=0;
        curr_chn=0;
        curr_chncol=0;

        sys_install_timer_isr();
        sys_set_timer_speed(SYS_TIMER_SPEED_50HZ);

        if(!(ned_phile=(char *)malloc(28000))) return(0);
        
        for(i=0; i<8; i++) {
          for(j=0; j<13; j++) {
            note_phreq[i][j]=(float)(((double)BASE_PHREQ*pow(2, (double)i)*pow(HTONE_CONST, (double)(j-1))));
            }
          }

        
        for(i=0; i<MAX_INSTRUMENTS; i++) {
          //memset(&inst[i], 1, sizeof(struct inst_struc));
          inst[i].duty_cycle=0;
          inst[i].hold_note=0;
          inst[i].envelope_phixed=0;
          inst[i].volume=0;
          inst[i].variable_phreq=0;
          inst[i].phq_changespd=0;
          inst[i].ishi2lo=0;
          inst[i].phq_range=0;
          inst[i].active_tlength=0;
          }

       
        for(i=0; i<MAX_CHANNELS; i++) {
          ChnPattern[i]=(ChannelPattern *)malloc(sizeof(struct snote_struc)*64*MAX_PATTERNS);
          memset(ChnPattern[i], 0, (sizeof(struct snote_struc)*64*MAX_PATTERNS));
          }

        // Allocate & clear DPCM Pattern
        //SamplePattern=(PatternDPCM *)malloc(sizeof(SnoteDPCM)*64*MAX_PATTERNS_DPCM);
        //memset(SamplePattern, 0, (sizeof(SnoteDPCM)*64*MAX_PATTERNS_DPCM));
        memset(OrderList, 0, sizeof(OrderList));
        return(1);
        }

#pragma aux get_key =   \
"xor     eax, eax"      \
"mov     ah, 1"         \
"int     16h"           \
"jz      buphpher_empty"\
"mov     ah, 0"         \
"int     16h"           \
"mov     al, ah"        \
"xor     ah, ah"        \
"jmp     read_done"     \
"buphpher_empty:"       \
"xor     eax, eax"      \
"read_done:"            \
modify  [eax]           \
value   [al];

        


/*unsigned char   get_key(void)
        {
        unsigned char result;
        _asm {
                mov     ah, 1
                int     16h
                jz      buphpher_empty
                mov     ah, 0
                int     16h
                mov     al, ah
                xor     ah, ah
                jmp     read_done
        buphpher_empty:
                xor     ax, ax
                read_done:
                mov     result, al
                }
        return(result);
        }*/


void    nt_update_pos(void)
        {
        int i, j;
        
        key_pressed=get_key();
        sys_do_misc_keys();


        if((sys_lepht_shipht_pressed) && ((key_pressed==KEY_JUMP) || (key_pressed==KEY_NUMASTERISK) || (key_pressed==KEY_UPLEPHT)))
          if((Editor++)>=SAMPLE_EDITOR) Editor=PATTERN_EDITOR;

         if((sys_lepht_ctrl_pressed) && (key_pressed==KEY_G))
           {
           unsigned char temp_key;
           scrtxt_put_pic_lines_clear(greet_list, 47);
           scrall_wait_vr();
           scrtxt_copy2vidmem();
           while(get_key()==KEY_G);
           while(get_key()==0);
           }

         
        if((sys_lepht_ctrl_pressed) && (key_pressed==KEY_P))
           {
           unsigned char temp_key;
           scrtxt_put_pic_lines_clear(porno_pic, 50);
           scrall_wait_vr();
           scrtxt_copy2vidmem();
           while(get_key()==KEY_P);
           while(get_key()==0);
           }


        if((!sys_lepht_shipht_pressed) && (!sys_right_shipht_pressed) &&
           (!sys_lepht_alt_pressed) && (!sys_right_alt_pressed) &&
           (!sys_lepht_ctrl_pressed) && (!sys_right_ctrl_pressed))
  {

        if(((int)key_pressed)==KEY_ESC)
                {
                unsigned char yn_key=0;

                scrall_wait_vr();
                nt_print_alert(exit_warns[rand()%NUMBER_OPH_WARNS]);
                scrtxt_copy2vidmem();

                //scrtxt_set_cursor_pos((long)(curr_chn*11+real_colpos[curr_chncol]), (long)CURSOR_LINE_SCR);
                cursor_blink=1;
                scrtxt_set_cursor_pos(0, 0);
                
                while(1)
                  {
                  yn_key=get_key();
                  if(yn_key==KEY_Y) {done=1; break;}
                  if((yn_key==KEY_N) || (yn_key==KEY_ESC)) break;
                  }
                cursor_blink=0;
                }


        if(Editor==PATTERN_EDITOR) 
        if((key_pressed==KEY_JUMP) || (key_pressed==KEY_NUMASTERISK))
          if((++edit_state)>2) edit_state=0;
        
        
        if((key_pressed==KEY_BSLASH) || (key_pressed==KEY_PH10)) if((++curr_inst)>MAX_INSTRUMENTS) curr_inst=MAX_INSTRUMENTS;
        if((key_pressed==KEY_APOS) || (key_pressed==KEY_PH9)) if((--curr_inst)<1) curr_inst=1;
        
        if(key_pressed==KEY_PH1) curr_octave=0;
        if(key_pressed==KEY_PH2) curr_octave=1;
        if(key_pressed==KEY_PH3) curr_octave=2;
        if(key_pressed==KEY_PH4) curr_octave=3;
        if(key_pressed==KEY_PH5) curr_octave=4;
        if(key_pressed==KEY_PH6) curr_octave=5;
        if(key_pressed==KEY_PH7) curr_octave=6;
        if(key_pressed==KEY_PH8) curr_octave=7;
        

       if(Editor==PATTERN_EDITOR) {
        if((key_pressed==KEY_PLUS) || (key_pressed==KEY_NUMPLUS)) {
          if(curr_chn==DPCM_CHN) {
                INC_LIM(OrderList[curr_order][curr_chn], (MAX_PATTERNS_DPCM-1));
                }
          else {
                INC_LIM(OrderList[curr_order][curr_chn], (MAX_PATTERNS-1));
               }
          }
        if((key_pressed==KEY_MINUS) || (key_pressed==KEY_NUMMINUS))
          DEC_LIM(OrderList[curr_order][curr_chn], 0);
        }
if(Editor==SAMPLE_EDITOR)
        nt_UpdatePosSampleEditor();


else if(Editor==PATTERN_EDITOR) {
        if(edit_state==EDIT_PAT)        // If editing patterns
                nt_update_pos_pat();        
        if(edit_state==EDIT_INST)
                nt_update_pos_inst();

        if(edit_state==EDIT_ORDER)
          {
          if(key_pressed==KEY_DOWN) {
            INC_LIM(curr_order, order_tot);
            curr_pat=order_list[curr_order];}
          if(key_pressed==KEY_UP) {
            DEC_LIM(curr_order, 0);
            curr_pat=order_list[curr_order];}

        /* if((key_pressed==KEY_PLUS) || (key_pressed==KEY_NUMPLUS))
          INC_LIM(curr_pat, (MAX_PATTERNS-1));
        if((key_pressed==KEY_MINUS) || (key_pressed==KEY_NUMMINUS))
          DEC_LIM(curr_pat, 0);*/

        if(key_pressed==KEY_SPACE)
          RestartPosition=curr_order;


          if(key_pressed==KEY_RIGHT)
            if((++curr_chn)>(MAX_CHANNELS-1))  { curr_chn=0;  curr_chncol=0;}
          //  INC_LIM(OrderList[curr_order][curr_chn], (MAX_PATTERNS-1));
          if(key_pressed==KEY_LEPHT)
            if((--curr_chn)<0) { curr_chn=(MAX_CHANNELS-1); curr_chncol=NUMBER_OPH_COLS-1;}
          //  DEC_LIM(OrderList[curr_order][curr_chn], 0);




          if(key_pressed==KEY_INS) {
            INC_LIM(order_tot, (128-1));
            for(i=order_tot; i>curr_order; i--) {
              CopyOrderEntry((i-1), i);
              }
            ClearOrderEntry(curr_order);
            }
            

          if(key_pressed==KEY_DEL)
            {
            DEC_LIM(order_tot, 1);
            if(order_tot>1)
              {
              for(i=curr_order; i<order_tot; i++)
                {
                CopyOrderEntry(i+1, i);
                ClearOrderEntry(i+1);
                }
              }
            }
          } // End iph == EDIT_ORDER
        } // End if in pattern editor
  }
        }


void    nt_print_alert(char *string)
        {
        int     i, str_length;
        str_length=strlen(string);

        cursor_blink=1;
        scrtxt_set_cursor_pos(0, 0);
        
        for(i=1; i<79; i++) {
                scrtxt_write_char(i, ALERT_LINE-1, 223, 15, 7, 0);
                scrtxt_write_char(i, ALERT_LINE, ' ', 15, 7, 0);
                scrtxt_write_char(i, ALERT_LINE+1, 220, 15, 7, 0);
                }
        for(i=0; i<str_length; i++) {
                scrtxt_write_char((40-(str_length>>1)+i), ALERT_LINE, string[i], 0, 7, 0);
                }
        cursor_blink=0;
        }

void    nt_update_pos_pat(void)
        {
        int     i;
                if(key_pressed==KEY_LEPHT)
                        DEC_COL(1);

                if(key_pressed==KEY_RIGHT)
                        INC_COL(1);

                if(key_pressed==KEY_UP) DEC_ROW(1);
                if(key_pressed==KEY_DOWN) INC_ROW(1);

                
                if(key_pressed==KEY_TAB)
                  {
                  if((++curr_chn)>=MAX_CHANNELS) curr_chn=0;
                  else curr_chncol=0;
                  }

                
                if(key_pressed==KEY_INS)
                  {
                  for(i=62; i>=curr_row; i--)
                    memcpy(&ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[i+1], &ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[i], sizeof(struct snote_struc));
                  memset(&ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row], 0, sizeof(struct snote_struc));
                  }
                
                if(key_pressed==KEY_BKSPACE)
                  {
                  if(curr_row)
                  {
                  DEC_ROW(1);
                  for(i=curr_row; i<63; i++)
                    memcpy(&ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[i], &ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[i+1], sizeof(struct snote_struc));
                  memset(&ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[63], 0, sizeof(struct snote_struc));
                  }
                  }
                
                
                if(key_pressed==KEY_DEL)
                  {
                  snote_struc &Snote=ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row];
                  //if((curr_chncol==NOTE_COL) || (curr_chncol==OCTAVE_COL)) {
                  if((curr_chncol==NOTE_COL) || (curr_chncol==OCTAVE_COL) ||
                     (curr_chncol==INST_COL1) || (curr_chncol==INST_COL2)) {
                    Snote.note=0;
                    Snote.octave=0;
                  //  }
                  //if((curr_chncol==INST_COL1) || (curr_chncol==INST_COL2)) {
                    Snote.inst_num1=0;
                    Snote.inst_num2=0;
                    }
                  if((curr_chncol==EPHPHECT_COL) || (curr_chncol==EPARM1_COL) || (curr_chncol==EPARM2_COL)) {
                    Snote.ephphect=0;
                    Snote.e_parm1=0;
                    Snote.e_parm2=0;
                    }

                  //memset(&ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row], 0, sizeof(struct snote_struc));
                  INC_ROW(1);
                  }
                
                if(key_pressed==KEY_PGDOWN) INC_ROW(16);
                if(key_pressed==KEY_PGUP) DEC_ROW(16);

                if(key_pressed==KEY_HOME) curr_row=0;
                if(key_pressed==KEY_END) curr_row=63;
                
                // Do diphpherent things depending on current column
                if(curr_chncol==NOTE_COL)
                  {
                  if(key_pressed==KEY_NOTE_Cn1) {
                        nt_write_note(NOTE_C, 1);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_Dn1) {
                        nt_write_note(NOTE_D, 1);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_En1) {
                        nt_write_note(NOTE_E, 1);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_PHn1) {
                        nt_write_note(NOTE_PH, 1);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_Gn1) {
                        nt_write_note(NOTE_G, 1);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_An1) {
                        nt_write_note(NOTE_A, 1);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_Bn1) {
                        nt_write_note(NOTE_B, 1);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Cs1) {
                        nt_write_note(NOTE_Cs, 1);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Ds1) {
                        nt_write_note(NOTE_Ds, 1);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_PHs1) {
                        nt_write_note(NOTE_PHs, 1);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Gs1) {
                        nt_write_note(NOTE_Gs, 1);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_As1) {
                        nt_write_note(NOTE_As, 1);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Cn1) {
                        nt_write_note(12, 1);           // ???!!!
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Dn2) {
                        nt_write_note(NOTE_D, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_En2) {
                        nt_write_note(NOTE_E, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_PHn2) {
                        nt_write_note(NOTE_PH, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Gn2) {
                        nt_write_note(NOTE_G, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_An2) {
                        nt_write_note(NOTE_A, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Bn2) {
                        nt_write_note(NOTE_B, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Cn2) {
                        nt_write_note(NOTE_C, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Dn3) {
                        nt_write_note(NOTE_D, 3);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_En3) {
                        nt_write_note(NOTE_E, 3);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Cs2) {
                        nt_write_note(NOTE_Cs, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Ds2) {
                        nt_write_note(NOTE_Ds, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_PHs2) {
                        nt_write_note(NOTE_PHs, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Gs2) {
                        nt_write_note(NOTE_Gs, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_As2) {
                        nt_write_note(NOTE_As, 2);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Cn3) {
                        nt_write_note(NOTE_C, 3);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_Cs3) {
                        nt_write_note(NOTE_Cs, 3);
                        INC_ROW(1);}
                        
                  else if(key_pressed==KEY_NOTE_Dn3) {
                        nt_write_note(NOTE_D, 3);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_Ds3) {
                        nt_write_note(NOTE_Ds, 3);
                        INC_ROW(1);}

                  else if(key_pressed==KEY_NOTE_En3) {
                        nt_write_note(NOTE_E, 3);
                        INC_ROW(1);}

                  /*else if(key_pressed==KEY_NOTE_NO) {
                        nt_write_note(NOTE_NO, 0);
                        INC_ROW(1);}*/

                  else if(key_pressed==KEY_NOTE_OPHPH) {
                        nt_write_note(NOTE_OPHPH, 0);
                        INC_ROW(1);}
                  }
        else if(curr_chncol==OCTAVE_COL)
          {
          if(scan2hexdig(key_pressed, 7)!=STD_ERROR)
            {
            if(ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].note) {
              ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].octave=scan2hexdig(key_pressed, 7);
              INC_ROW(1);}
            }
          }
        else if(curr_chncol==INST_COL1)
          {
          if(scan2hexdig(key_pressed, 1)!=STD_ERROR)
            {
            //if(ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].note) {
              ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].inst_num1=scan2hexdig(key_pressed, 1);
              INC_ROW(1);}
            //}
          }
        else if(curr_chncol==INST_COL2)
          {
          if(scan2hexdig(key_pressed, 15)!=STD_ERROR)
            {
            //if(ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].note) {
              ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].inst_num2=scan2hexdig(key_pressed, 15);
              INC_ROW(1);}
            //}
          }
        else if(curr_chncol==EPHPHECT_COL)
          {
          if(scan2hexdig(key_pressed, 15)!=STD_ERROR)
            {
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].ephphect=scan2hexdig(key_pressed, 15);
          if(!scan2hexdig(key_pressed, 15)) {
            ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].e_parm1=0;
            ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].e_parm2=0;}
            INC_ROW(1);
            }
          }
        else if(curr_chncol==EPARM1_COL)
          {
          if((scan2hexdig(key_pressed, 15)!=STD_ERROR) &&
          (ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].ephphect))
            {
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].e_parm1=scan2hexdig(key_pressed, 15);
            INC_ROW(1);
            }
          }
        else if(curr_chncol==EPARM2_COL)
          {
          if((scan2hexdig(key_pressed, 15)!=STD_ERROR) &&
          (ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].ephphect))
            {
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].e_parm2=scan2hexdig(key_pressed, 15);
            INC_ROW(1);
            }
          }
      }


void    nt_update_pos_inst(void)
        {

          if(key_pressed==KEY_DOWN)
            INC_LIM(curr_instrow, 22);
          if(key_pressed==KEY_UP)
            DEC_LIM(curr_instrow, 0);
          
          if(key_pressed==KEY_LEPHT)
            {
            if(curr_instrow==0) DEC_LIM(inst[curr_inst-1].duty_cycle, 0);
            if(curr_instrow==1) DEC_LIM(inst[curr_inst-1].hold_note, 0);
            if(curr_instrow==2) DEC_LIM(inst[curr_inst-1].envelope_phixed, 0);
            if(curr_instrow==3) DEC_LIM(inst[curr_inst-1].volume, 0);
            if(curr_instrow==4) DEC_LIM(inst[curr_inst-1].variable_phreq, 0);
            if(curr_instrow==5) DEC_LIM(inst[curr_inst-1].phq_changespd, 0);
            if(curr_instrow==6) DEC_LIM(inst[curr_inst-1].ishi2lo, 0);
            if(curr_instrow==7) DEC_LIM(inst[curr_inst-1].phq_range, 0);
            if(curr_instrow==8) DEC_LIM(inst[curr_inst-1].active_tlength, 0);

            if(curr_instrow==9) DEC_LIM(inst[curr_inst-1].ArpX, 0);
            if(curr_instrow==10) DEC_LIM(inst[curr_inst-1].ArpY, 0);
            if(curr_instrow==11) DEC_LIM(inst[curr_inst-1].ArpZ, 0);

            if(curr_instrow==12) DEC_LIM(inst[curr_inst-1].VibratoSpeed, 0);
            if(curr_instrow==13) DEC_LIM(inst[curr_inst-1].VibratoDepth, 0);

            if(curr_instrow==14) DEC_LIM(inst[curr_inst-1].TremoloSpeed, 0);
            if(curr_instrow==15) DEC_LIM(inst[curr_inst-1].TremoloDepth, 0);
                               
            if(curr_instrow==16) DEC_LIM(inst[curr_inst-1].VolumeSlide, 0);
            if(curr_instrow==17) DEC_LIM(inst[curr_inst-1].VolumeSlideDir, 0);

            if(curr_instrow==18) DEC_LIM(inst[curr_inst-1].LoopedNoise, 0);

            if(curr_instrow==19) DEC_LIM(inst[curr_inst-1].Porta, 0);
            if(curr_instrow==20) DEC_LIM(inst[curr_inst-1].PortaDir, 0);

            if(curr_instrow==21) DEC_LIM(inst[curr_inst-1].ReversedArpeggio, 0);
            if(curr_instrow==22) DEC_LIM(inst[curr_inst-1].NonLoopedArpeggio, 0);
            }
          if(key_pressed==KEY_RIGHT)
            {
            if(curr_instrow==0) INC_LIM(inst[curr_inst-1].duty_cycle, 3);
            if(curr_instrow==1) INC_LIM(inst[curr_inst-1].hold_note, 1);
            if(curr_instrow==2) INC_LIM(inst[curr_inst-1].envelope_phixed, 1);
            if(curr_instrow==3) INC_LIM(inst[curr_inst-1].volume, 15);
            if(curr_instrow==4) INC_LIM(inst[curr_inst-1].variable_phreq, 1);
            if(curr_instrow==5) INC_LIM(inst[curr_inst-1].phq_changespd, 7);
            if(curr_instrow==6) INC_LIM(inst[curr_inst-1].ishi2lo, 1);
            if(curr_instrow==7) INC_LIM(inst[curr_inst-1].phq_range, 7);
            if(curr_instrow==8) INC_LIM(inst[curr_inst-1].active_tlength, 0x3F);
            if(curr_instrow==9) INC_LIM(inst[curr_inst-1].ArpX, 0xF);
            if(curr_instrow==10) INC_LIM(inst[curr_inst-1].ArpY, 0xF);
            if(curr_instrow==11) INC_LIM(inst[curr_inst-1].ArpZ, 0xF);

            if(curr_instrow==12) INC_LIM(inst[curr_inst-1].VibratoSpeed, 0xF);
            if(curr_instrow==13) INC_LIM(inst[curr_inst-1].VibratoDepth, 0xF);

            if(curr_instrow==14) INC_LIM(inst[curr_inst-1].TremoloSpeed, 0xF);
            if(curr_instrow==15) INC_LIM(inst[curr_inst-1].TremoloDepth, 0xF);

            if(curr_instrow==16) INC_LIM(inst[curr_inst-1].VolumeSlide, 0xF);
            if(curr_instrow==17) INC_LIM(inst[curr_inst-1].VolumeSlideDir, 1);

            if(curr_instrow==18) INC_LIM(inst[curr_inst-1].LoopedNoise, 1);

            if(curr_instrow==19) INC_LIM(inst[curr_inst-1].Porta, 0x7F);
            if(curr_instrow==20) INC_LIM(inst[curr_inst-1].PortaDir, 1);

            if(curr_instrow==21) INC_LIM(inst[curr_inst-1].ReversedArpeggio, 1);
            if(curr_instrow==22) INC_LIM(inst[curr_inst-1].NonLoopedArpeggio, 1);
            }
        }

void    PlayNote(int Chn, int Inst, int Note, int Oct)
        {

        if(Chn==DPCM_CHN) {
                BYTE    r4010, r4011, r4012, r4013;

                int Samp=(NoteTable[Inst-1][Oct][Note-1]>>4) & 0x7;
                if(!SampleInst[Inst-1].Sample[Samp].SamplePtr) return;;
                UpperPrgPage[0]=(void *)SampleInst[Inst-1].Sample[Samp].SamplePtr;
                UpperPrgPage[1]=UpperPrgPage[0];
                UpperPrgPage[2]=UpperPrgPage[0];
                UpperPrgPage[3]=UpperPrgPage[0];
                r4010=(NoteTable[Inst-1][Oct][Note-1] & 0xF) | (SampleInst[Inst-1].Sample[Samp].LoopFlag<<6);
                r4011=SampleInst[Inst-1].Sample[Samp].Volume;
                r4012=0x00;
                r4013=SampleInst[Inst-1].Sample[Samp].Length;

                Write2SoundReg(0x4010, r4010);
                Write2SoundReg(0x4011, r4011);
                Write2SoundReg(0x4012, r4012);
                Write2SoundReg(0x4013, r4013);

                Write2SoundReg(0x4015, (Reg4015 & 0xF));
                Reg4015|=0x10;
                Write2SoundReg(0x4015, Reg4015);
                }
        else {
        BYTE SndReg0, SndReg1, SndReg2, SndReg3;
        int  Tone=(Oct<<3)+(Oct<<2)+Note;
          SndReg0=(inst[Inst-1].duty_cycle<<6)+(inst[Inst-1].hold_note<<5)+
                (inst[Inst-1].envelope_phixed<<4)+(inst[Inst-1].volume);
          SndReg1=(inst[Inst-1].variable_phreq<<7)+(inst[Inst-1].phq_changespd<<4)+
                (inst[Inst-1].ishi2lo<<3)+(inst[Inst-1].phq_range);
          SndReg2=Note2PeriodTab[Tone-1] & 0xFF;
          SndReg3=(inst[Inst-1].active_tlength<<3) | (Note2PeriodTab[Tone-1]>>8);

        Reg4015|=(1<<Chn);
        Write2SoundReg(0x4015, Reg4015);

        Write2SoundReg(0x4000+Chn*4, SndReg0);
        Write2SoundReg(0x4001+Chn*4, SndReg1);
        Write2SoundReg(0x4002+Chn*4, SndReg2);
        Write2SoundReg(0x4003+Chn*4, SndReg3);
        }

        // Write2SoundReg(0x4000, 0x0F);
        // Write2SoundReg(0x4001, 0x00);
        // Write2SoundReg(0x4002, 0x5F);
        // Write2SoundReg(0x4003, 0x01);

        }

void    nt_write_note(int note, int add_octave)
        {
        int     phinal_octave;

          phinal_octave=((curr_octave+add_octave) &0x7);
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].note=note;
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].octave=phinal_octave;
        if(note!=NOTE_OPHPH) {
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].inst_num1=NIB_HI(curr_inst);
          ChnPattern[curr_chn][OrderList[curr_order][curr_chn]].Row[curr_row].inst_num2=NIB_LO(curr_inst);
          }
        if(note!=NOTE_NO)
          {
          // sound((int)note_phreq[phinal_octave][note]);
          // nt_sound_delay=sys_phrame_counter;
          PlayNote(curr_chn, curr_inst, note, phinal_octave);
          }

        }

int     scan2hexdig(char   scan_code, char limit)
        {
        int     digit;
        if(scan_code==KEY_0) digit=0;
        else if(scan_code==KEY_1) digit=1;
        else if(scan_code==KEY_2) digit=2;
        else if(scan_code==KEY_3) digit=3;
        else if(scan_code==KEY_4) digit=4;
        else if(scan_code==KEY_5) digit=5;
        else if(scan_code==KEY_6) digit=6;
        else if(scan_code==KEY_7) digit=7;
        else if(scan_code==KEY_8) digit=8;
        else if(scan_code==KEY_9) digit=9;
        else if(scan_code==KEY_A) digit=0xA;
        else if(scan_code==KEY_B) digit=0xB;
        else if(scan_code==KEY_C) digit=0xC;
        else if(scan_code==KEY_D) digit=0xD;
        else if(scan_code==KEY_E) digit=0xE;
        else if(scan_code==KEY_PH) digit=15;
        else return(STD_ERROR);

        if(digit>limit) return(STD_ERROR);
        else    return(digit);
        }



void    nt_UpdatePosSampleEditor(void)
        {
        if((key_pressed==KEY_JUMP) || (key_pressed==KEY_NUMASTERISK))
          if((++SampleEditorState)>EDIT_SAMPLE_SETTINGS) SampleEditorState=EDIT_SAMPLE_NAMES;
        if(SampleEditorState==EDIT_SAMPLE_SETTINGS)
                nt_UpdatePosSampleSettings();
        if(SampleEditorState==EDIT_SAMPLE_NAMES)
                nt_UpdatePosInstNames();
        if(SampleEditorState==EDIT_SAMPLE_SNAMES)
                nt_UpdatePosSampleNames();
        if(SampleEditorState==EDIT_SAMPLE_NOTETABLE)
                nt_UpdatePosNoteTable();
        CurrSampleInst=FirstIname+InameWindowLine+1;
        }


void    nt_UpdatePosSampleSettings(void)
        {
        if(key_pressed==KEY_DOWN)
          INC_LIM(CurrSampleSetting, 3);
        if(key_pressed==KEY_UP)
          DEC_LIM(CurrSampleSetting, 0);

        if(key_pressed==KEY_LEPHT) {
          if(CurrSampleSetting==SAMPLE_SETTING_VOLUME)
            DEC_LIM(SampleInst[CurrSampleInst-1].Sample[CurrSample].Volume, 0);
          if(CurrSampleSetting==SAMPLE_SETTING_REPEAT)
            DEC_LIM(SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopStart, 0);
          if(CurrSampleSetting==SAMPLE_SETTING_LOOPFLAG)
            SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopFlag=((SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopFlag+1) & 1);

          //if(CurrSampleSetting==SAMPLE_SETTING_REPLEN)
          //  DEC_LIM(SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopLength, 0);

          }
        if(key_pressed==KEY_RIGHT) {
          if(CurrSampleSetting==SAMPLE_SETTING_VOLUME)
            INC_LIM(SampleInst[CurrSampleInst-1].Sample[CurrSample].Volume, 0x7F);
          if(CurrSampleSetting==SAMPLE_SETTING_REPEAT)
            INC_LIM(SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopStart, 0x3F);
          if(CurrSampleSetting==SAMPLE_SETTING_LOOPFLAG)
            SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopFlag=((SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopFlag+1) & 1);

          //if(CurrSampleSetting==SAMPLE_SETTING_REPLEN)
          //  INC_LIM(SampleInst[CurrSampleInst-1].LoopLength, 0xFF);
          }

        }

void    nt_UpdatePosNoteTable(void)
        {
        if(key_pressed==KEY_LEPHT) {
          if(NoteTableSmpOrFreq) NoteTableSmpOrFreq--;
          else
            if(CurrNoteTableOctave>0) {
                CurrNoteTableOctave--;
                NoteTableSmpOrFreq=1;}
          }
        if(key_pressed==KEY_RIGHT) {
          if(!NoteTableSmpOrFreq) NoteTableSmpOrFreq++;
          else
            if((CurrNoteTableOctave+1)<MAX_DPCM_OCTAVES) {
                CurrNoteTableOctave++;
                NoteTableSmpOrFreq=0;}
          }

        if(key_pressed==KEY_UP)
          DEC_LIM(CurrNoteTableNote, 0);

        if(key_pressed==KEY_DOWN)
          INC_LIM(CurrNoteTableNote, 11);

        {
        BYTE    *NoteTableEntry=&NoteTable[CurrSampleInst-1][CurrNoteTableOctave][CurrNoteTableNote];
        int     ToneSample=((NoteTableEntry[0]>>4) & 0x7);
        int     ToneFreq=(NoteTableEntry[0] & 0xF);
        if((key_pressed==KEY_PLUS) || (key_pressed==KEY_NUMPLUS)) {
          if(NoteTableSmpOrFreq)
            INC_LIM(ToneFreq, 0xF);
          if(!NoteTableSmpOrFreq)
            INC_LIM(ToneSample, 0x7);
          }
        if((key_pressed==KEY_MINUS) || (key_pressed==KEY_NUMMINUS)) {
          if(NoteTableSmpOrFreq)
            DEC_LIM(ToneFreq, 0);
          if(!NoteTableSmpOrFreq)
            DEC_LIM(ToneSample, 0);
          }
        NoteTableEntry[0]=(ToneSample<<4)+ToneFreq;
        }
        }

void    nt_UpdatePosInstNames(void)
        {
        if(key_pressed==KEY_DOWN) {
          if((InameWindowLine+1)>=NUM_VISIBLE_INAMES) {
            if((FirstIname+NUM_VISIBLE_INAMES+1)<=MAX_INSTRUMENTS_DPCM)
                FirstIname++;
            }
          else InameWindowLine++;
          }
          //INC_LIM(CurrSampleInst, MAX_INSTRUMENTS_DPCM);
        if(key_pressed==KEY_UP) {
          if((InameWindowLine-1)<0) {
            if((FirstIname-1)>=0)
                FirstIname--;
            }
          else InameWindowLine--;
          }

          //DEC_LIM(CurrSampleInst, 1);
        //if(key_pressed==KEY_RIGHT)
        //  if((CurrSampleInst+=8)>MAX_INSTRUMENTS_DPCM) CurrSampleInst=MAX_INSTRUMENTS_DPCM;
        //if(key_pressed==KEY_LEPHT)
        //  if((CurrSampleInst-=8)<1) CurrSampleInst=1;
        }

void    nt_UpdatePosSampleNames(void)
        {
        if(key_pressed==KEY_DOWN)
          INC_LIM(CurrSample, (MAX_DPCM_SAMPLES-1));
        if(key_pressed==KEY_UP)
          DEC_LIM(CurrSample, 0);
        if(key_pressed==KEY_L) {
                int     i, j;
                char    SampleFileName[80];
                long    SampleLength;

                SelectFile(SampleFileName, "*.*");
                if((SampleLength=get_filesize(SampleFileName))>4081)
                        SampleLength=4081;
                SampleLength=DPCMLength2Normal(Normal2DPCMLength(SampleLength));
                if(SampleInst[CurrSampleInst-1].Sample[CurrSample].SamplePtr)
                        free(SampleInst[CurrSampleInst-1].Sample[CurrSample].SamplePtr);
                SampleInst[CurrSampleInst-1].Sample[CurrSample].SamplePtr=(BYTE *)malloc(SampleLength);
                readfile(SampleFileName, 0, SampleLength, SampleInst[CurrSampleInst-1].Sample[CurrSample].SamplePtr);
                SampleInst[CurrSampleInst-1].Sample[CurrSample].Length=Normal2DPCMLength(SampleLength);
                for(i=0; ((i<DPCM_SNAME_LENGTH) && (SampleFileName[i])); i++)
                        SampleName[CurrSampleInst-1][CurrSample][i]=SampleFileName[i];
                while(i<DPCM_SNAME_LENGTH) {
                        SampleName[CurrSampleInst-1][CurrSample][i]=NULL;
                        i++;}
                }
        else
          if(key_pressed==KEY_DEL) {
                SampleDPCM *Samp=&SampleInst[CurrSampleInst-1].Sample[CurrSample];
                free((*Samp).SamplePtr);
                (*Samp).Length=0;
                (*Samp).LoopStart=0;
                (*Samp).LoopFlag=0;
                (*Samp).LoopAmp=0;
                (*Samp).LoopStart=0;
                (*Samp).Volume=0;
                memset(SampleName[CurrSampleInst-1][CurrSample], 0, DPCM_SNAME_LENGTH);
                }
        }


void nt_draw_misc(void)
  {
  int   i, j, k;
  if(Editor==PATTERN_EDITOR) {
    if((edit_state==EDIT_PAT)) cursor_blink=0;
    else cursor_blink=1;
    }
  if(Editor==SAMPLE_EDITOR) {
    cursor_blink=1;
    }
    
  scrtxt_set_cursor_pos((long)(curr_chn*11+real_colpos[curr_chncol]), (long)CURSOR_LINE_SCR);
          
  draw_insted();
  
  if(Editor==PATTERN_EDITOR) {
    draw_order();
    DrawPatternEditor();
    }
  else if(Editor==SAMPLE_EDITOR) {
    DrawSampleEditor();
    }
  //draw_moremisc();
  
  }


void    DrawPatternEditor(void)
        {
        int     i, j, k;
        int   Lit;
        int   temp_patrow=(int)curr_row-(CURSOR_LINE_SCR-PATLINES_START_SCR),
              temp_patrow_scr=PATLINES_START_SCR;
        int   DisplayedPatlines=PATLINES_END_SCR-PATLINES_START_SCR;
        char  SnoteString[SNOTE_LENGTH_SCR];
        BYTE  SnoteColor[SNOTE_LENGTH_SCR];
        char  SnoteDPCMString[SNOTE_DPCM_LENGTH_SCR];
        BYTE  SnoteDPCMColor[SNOTE_DPCM_LENGTH_SCR];
        const   int VuMetersX=58,
                VuMetersY=11,
                VuMeterHeight=6;

        for(i=0; i<NUM_CHANNELS; i++) {
          int   VuHeight=(abs(ChannelAmplitude[i])*VuMeterHeight)>>15;
          int   BlankRows=VuMeterHeight-VuHeight;

          if(!IsPlaying)
            VuHeight=VuMeterHeight;
          else
            for(j=0; j<BlankRows; j++) {
              scrtxt_vidbuphph[((VuMetersY-VuMeterHeight+j)*80+(VuMetersX+i*4))*2]=0;
              scrtxt_vidbuphph[((VuMetersY-VuMeterHeight+j)*80+(VuMetersX+i*4))*2+1]=0;
              scrtxt_vidbuphph[((VuMetersY-VuMeterHeight+j)*80+(VuMetersX+i*4)+1)*2]=0;
              scrtxt_vidbuphph[((VuMetersY-VuMeterHeight+j)*80+(VuMetersX+i*4)+1)*2+1]=0;
              }
          }


        for(i=0; i<DisplayedPatlines; i++, temp_patrow++, temp_patrow_scr++)
          {
    
          if(temp_patrow<(-1))
            write_54chars((long)0, (long)temp_patrow_scr, ' ', 15, 0);
          else if(temp_patrow==(-1)) write_upperbord(temp_patrow_scr);
          else if(temp_patrow==64) write_lowerbord(temp_patrow_scr);
          else if(temp_patrow>=0 && temp_patrow<64)
            {
            if((!(temp_patrow & 0x07)) && (temp_patrow & 0xF8)) Lit=1;
            else Lit=0;
            for(j=0; j<MAX_CHANNELS; j++) {
              GetSnoteString(&ChnPattern[j][OrderList[curr_order][j]].Row[temp_patrow], SnoteString, SnoteColor, Lit);
              for(k=0; k<SNOTE_LENGTH_SCR; k++) {
                scrtxt_vidbuphph[temp_patrow_scr*160+((j*(SNOTE_LENGTH_SCR+1))+k)*2]=SnoteString[k];
                scrtxt_vidbuphph[temp_patrow_scr*160+((j*(SNOTE_LENGTH_SCR+1))+k)*2+1]=SnoteColor[k];
                }
              }
            //GetSnoteDPCMString(&SamplePattern[OrderList[curr_order][DPCM_CHN]].Row[temp_patrow], SnoteDPCMString, SnoteDPCMColor, Lit);
            //for(k=0; k<SNOTE_LENGTH_SCR; k++) {
            //  scrtxt_vidbuphph[temp_patrow_scr*160+((j*(SNOTE_DPCM_LENGTH_SCR+1))+k)*2]=SnoteDPCMString[k];
            //  scrtxt_vidbuphph[temp_patrow_scr*160+((j*(SNOTE_DPCM_LENGTH_SCR+1))+k)*2+1]=SnoteDPCMColor[k];
            //  }
            }
          else if(temp_patrow>64) write_54chars((long)0, (long)temp_patrow_scr, ' ', 15, 0);
          }
      
        //DrawBgColLine_H(0, (SNOTE_LENGTH_SCR+1)*MAX_CHANNELS-2, CURSOR_LINE_SCR, 0x04, scrtxt_vidbuphph);
        //DrawFgColLine_H(0, (SNOTE_LENGTH_SCR+1)*MAX_CHANNELS-2, CURSOR_LINE_SCR, 0x0F, scrtxt_vidbuphph);

        DrawBothColLine_H(0, (SNOTE_LENGTH_SCR+1)*MAX_CHANNELS-2, CURSOR_LINE_SCR, 0x4F, scrtxt_vidbuphph);
        
        // Printa Phlummets dumma pattern-visare...
        for(i=0; i<MAX_CHANNELS; i++) {
          scrtxt_write_char(4+(SNOTE_LENGTH_SCR+1)*i, 47, hex_tab[NIB_HI(OrderList[curr_order][i])], 0, 7, 0);
          scrtxt_write_char(5+(SNOTE_LENGTH_SCR+1)*i, 47, hex_tab[NIB_LO(OrderList[curr_order][i])], 0, 7, 0);
          }
        }


void    Word2HexStr(int Val, char *Str)
        {
        Str[0]=hex_tab[((Val>>12) & 0xF)];
        Str[1]=hex_tab[((Val>>8) & 0xF)];
        Str[2]=hex_tab[((Val>>4) & 0xF)];
        Str[3]=hex_tab[((Val>>0) & 0xF)];
        }

void    DrawSampleEditor(void)
        {
        int     i, j;
        Blit(0, 0, 80, 15, SampleWindows, scrtxt_vidbuphph);

        for(i=FirstIname; i<(FirstIname+NUM_VISIBLE_INAMES); i++) {
          scrtxt_write_char(INAMES_X-2, INAMES_Y+(i-FirstIname)-1, hex_tab[i+1], 15, 1, 0);
          if(strlen(InstName[i]))
            for(j=0; j<DPCM_INAME_LENGTH; j++)
              scrtxt_write_char(INAMES_X+j, INAMES_Y-1+(i-FirstIname), InstName[i][j], 15, 1, 0);
          }

        for(i=0; i<MAX_DPCM_SAMPLES; i++) {
          if(strlen(SampleName[CurrSampleInst-1][i]))
            for(j=0; j<DPCM_SNAME_LENGTH; j++)
              scrtxt_write_char(SNAMES_X+j, SNAMES_Y+i, SampleName[CurrSampleInst-1][i][j], 15, 1, 0);
          }
        


          {
          char  Stringie[5];
          int   SettingsTab[4];
          SettingsTab[0]=SampleInst[CurrSampleInst-1].Sample[CurrSample].Volume;
          if(SampleInst[CurrSampleInst-1].Sample[CurrSample].Length)
            SettingsTab[1]=DPCMLength2Normal(SampleInst[CurrSampleInst-1].Sample[CurrSample].Length);
          else SettingsTab[1]=0;
          SettingsTab[2]=SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopStart*64;
          //SettingsTab[3]=SampleInst[CurrSampleInst-1].LoopLength*16;
          //SettingsTab[3]=0x47;
          Stringie[4]=0;

          for(i=0; i<3; i++) {
            Word2HexStr(SettingsTab[i], Stringie);
            PrintStringBoth(SAMPLE_SETTINGS_X+12,4+i*2,Stringie,0x0F,scrtxt_vidbuphph);
            }
          if(SampleInst[CurrSampleInst-1].Sample[CurrSample].LoopFlag)
                scrtxt_write_char(SAMPLE_SETTINGS_X+14, 10, 'X', 15, 0, 0);
          else  scrtxt_write_char(SAMPLE_SETTINGS_X+14, 10, '-', 15, 0, 0);
          }
        //DrawWaveForm(&SampleInst[CurrSampleInst-1], WaveFormX1, WaveFormY1, WaveFormX2, WaveFormY2, 80);


        for(i=0; i<MAX_DPCM_OCTAVES; i++)
          for(j=0; j<12; j++) {
            scrtxt_write_char(NOTE_TABLE_X+i*3, NOTE_TABLE_Y+j, hex_tab[(NoteTable[CurrSampleInst-1][i][j]>>4) &0x7], 15, 1, 0);
            scrtxt_write_char(NOTE_TABLE_X+i*3+1, NOTE_TABLE_Y+j, hex_tab[NoteTable[CurrSampleInst-1][i][j] & 0xF], 15, 1, 0);
            }
        if(SampleEditorState==EDIT_SAMPLE_SETTINGS)
          DrawBothColLine_H(SAMPLE_SETTINGS_X, SAMPLE_SETTINGS_X+5, SAMPLE_SETTINGS_Y+CurrSampleSetting*2, 0x4F, scrtxt_vidbuphph);

          DrawBgColLine_H(INAMES_X-3, INAMES_X-2, INAMES_Y-1+InameWindowLine, 0x06, scrtxt_vidbuphph);
        if(SampleEditorState==EDIT_SAMPLE_NAMES)
          DrawBgColLine_H(INAMES_X, INAMES_X+DPCM_INAME_LENGTH, INAMES_Y-1+InameWindowLine, 0x04, scrtxt_vidbuphph);

          DrawBgColLine_H(SNAMES_X-2, SNAMES_X-2, INAMES_Y-2+CurrSample, 0x06, scrtxt_vidbuphph);
        if(SampleEditorState==EDIT_SAMPLE_SNAMES)
          DrawBgColLine_H(SNAMES_X, SNAMES_X+DPCM_INAME_LENGTH, INAMES_Y-2+CurrSample, 0x04, scrtxt_vidbuphph);

        if(SampleEditorState==EDIT_SAMPLE_NOTETABLE) {
          if(!NoteTableSmpOrFreq)
            DrawBothColLine_H(NOTE_TABLE_X+CurrNoteTableOctave*3, NOTE_TABLE_X+CurrNoteTableOctave*3, NOTE_TABLE_Y+CurrNoteTableNote, 0x4F, scrtxt_vidbuphph);
          if(NoteTableSmpOrFreq)
            DrawBothColLine_H(NOTE_TABLE_X+CurrNoteTableOctave*3+1, NOTE_TABLE_X+CurrNoteTableOctave*3+1, NOTE_TABLE_Y+CurrNoteTableNote, 0x4F, scrtxt_vidbuphph);
          }
        }

/*void    DrawWaveForm(InstDPCM *Inst, int x1, int y1, int x2, int y2, int LineWidth)
{
int     i, j, k;
int     WindowWidth=(x2-x1)+1,
        WindowHeight=(y2-y1)+1,
        WaveFormLength;
sBYTE   *WaveForm=NULL;
float   scale=WindowHeight/2;
        WaveFormLength=(Inst->Length+1)*16;
        WaveForm=(sBYTE *)malloc(WaveFormLength);
        Do1BitTo8Bit(Inst->SamplePtr, WaveForm);

float   Delta=(float)WindowWidth/WaveFormLength;
int     Row;

        for(i=0; i<WindowWidth; i++)
                Row=y1+WindowsHeight/2+(Scale*(float)(WaveForm[(int)(float)i*Delta)]));
                if((Row*2)%2)   scrtxt_write_char(0, Row, BLOCKCHAR_LOWER, 7, 0, 0);
                else            scrtxt_write_char(0, Row, BLOCKCHAR_LOWER, 7, 0, 0);
                }
        }*/

void write_54chars(long x, long y, char the_char, char color, char bcolor)
  {
  int i;
  for(i=0; i<54; i++)
    {
    scrtxt_write_char(x+(long)i, y, the_char, color, bcolor, 0);
    }
  }


void write_upperbord(int temp_patrow_scr)
  {
  int i;

    for(i=0; i<54; i++) scrtxt_write_char(i, temp_patrow_scr, 'Ä', 7, 0, 0);

    for(i=0; i<(MAX_CHANNELS-1); i++)
      scrtxt_write_char((long)i*11+10, (long)temp_patrow_scr, 'Â', 7, 0, 0);
  }

void write_lowerbord(int temp_patrow_scr)
  {
  int i;
    for(i=0; i<54; i++) scrtxt_write_char(i, temp_patrow_scr, 'Ä', 7, 0, 0);

    for(i=0; i<(MAX_CHANNELS-1); i++)
      scrtxt_write_char((long)i*11+10, (long)temp_patrow_scr, 'Á', 7, 0, 0);
  }


  
void draw_insted(void)
  {
  int   i;
  scrtxt_write_char(75, 17, hex_tab[NIB_HI(curr_inst)], 15, 0, 0);
  scrtxt_write_char(76, 17, hex_tab[NIB_LO(curr_inst)], 15, 0, 0);

  scrtxt_write_char(II_COL, II_ROW, hex_tab[NIB_HI(inst[curr_inst-1].duty_cycle)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW, hex_tab[NIB_LO(inst[curr_inst-1].duty_cycle)], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+1, boo_tab[inst[curr_inst-1].hold_note][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+1, boo_tab[inst[curr_inst-1].hold_note][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+1, boo_tab[inst[curr_inst-1].hold_note][2], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+2, boo_tab[inst[curr_inst-1].envelope_phixed][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+2, boo_tab[inst[curr_inst-1].envelope_phixed][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+2, boo_tab[inst[curr_inst-1].envelope_phixed][2], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL, II_ROW+3, hex_tab[NIB_HI(inst[curr_inst-1].volume)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+3, hex_tab[NIB_LO(inst[curr_inst-1].volume)], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+4, boo_tab[inst[curr_inst-1].variable_phreq][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+4, boo_tab[inst[curr_inst-1].variable_phreq][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+4, boo_tab[inst[curr_inst-1].variable_phreq][2], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL, II_ROW+5, hex_tab[NIB_HI(inst[curr_inst-1].phq_changespd)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+5, hex_tab[NIB_LO(inst[curr_inst-1].phq_changespd)], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+6, boo_tab[inst[curr_inst-1].ishi2lo][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+6, boo_tab[inst[curr_inst-1].ishi2lo][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+6, boo_tab[inst[curr_inst-1].ishi2lo][2], IIPH_COLOR, IIBK_COLOR, 0);
  
  scrtxt_write_char(II_COL, II_ROW+7, hex_tab[NIB_HI(inst[curr_inst-1].phq_range)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+7, hex_tab[NIB_LO(inst[curr_inst-1].phq_range)], IIPH_COLOR, IIBK_COLOR, 0);
  
  scrtxt_write_char(II_COL, II_ROW+8, hex_tab[NIB_HI(inst[curr_inst-1].active_tlength)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+8, hex_tab[NIB_LO(inst[curr_inst-1].active_tlength)], IIPH_COLOR, IIBK_COLOR, 0);
  
  scrtxt_write_char(II_COL+1, II_ROW+9, hex_tab[NIB_LO(inst[curr_inst-1].ArpX)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+10, hex_tab[NIB_LO(inst[curr_inst-1].ArpY)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+11, hex_tab[NIB_LO(inst[curr_inst-1].ArpZ)], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL+1, II_ROW+12, hex_tab[NIB_LO(inst[curr_inst-1].VibratoSpeed)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+13, hex_tab[NIB_LO(inst[curr_inst-1].VibratoDepth)], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL+1, II_ROW+14, hex_tab[NIB_LO(inst[curr_inst-1].TremoloSpeed)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+15, hex_tab[NIB_LO(inst[curr_inst-1].TremoloDepth)], IIPH_COLOR, IIBK_COLOR, 0);
  
  scrtxt_write_char(II_COL+1, II_ROW+16, hex_tab[NIB_LO(inst[curr_inst-1].VolumeSlide)], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+17, boo_tab[inst[curr_inst-1].VolumeSlideDir][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+17, boo_tab[inst[curr_inst-1].VolumeSlideDir][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+17, boo_tab[inst[curr_inst-1].VolumeSlideDir][2], IIPH_COLOR, IIBK_COLOR, 0);


  scrtxt_write_char(II_COL-1, II_ROW+18, boo_tab[inst[curr_inst-1].LoopedNoise][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+18, boo_tab[inst[curr_inst-1].LoopedNoise][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+18, boo_tab[inst[curr_inst-1].LoopedNoise][2], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL, II_ROW+19, hex_tab[NIB_HI(inst[curr_inst-1].Porta)], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+19, hex_tab[NIB_LO(inst[curr_inst-1].Porta)], IIPH_COLOR, IIBK_COLOR, 0);


  scrtxt_write_char(II_COL-1, II_ROW+20, boo_tab[inst[curr_inst-1].PortaDir][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+20, boo_tab[inst[curr_inst-1].PortaDir][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+20, boo_tab[inst[curr_inst-1].PortaDir][2], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+21, boo_tab[inst[curr_inst-1].ReversedArpeggio][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+21, boo_tab[inst[curr_inst-1].ReversedArpeggio][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+21, boo_tab[inst[curr_inst-1].ReversedArpeggio][2], IIPH_COLOR, IIBK_COLOR, 0);

  scrtxt_write_char(II_COL-1, II_ROW+22, boo_tab[inst[curr_inst-1].NonLoopedArpeggio][0], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL, II_ROW+22, boo_tab[inst[curr_inst-1].NonLoopedArpeggio][1], IIPH_COLOR, IIBK_COLOR, 0);
  scrtxt_write_char(II_COL+1, II_ROW+22, boo_tab[inst[curr_inst-1].NonLoopedArpeggio][2], IIPH_COLOR, IIBK_COLOR, 0);

  if(edit_state==EDIT_INST) for(i=0; i<22; i++) scrtxt_write_col(II_COL+i-20, II_ROW+curr_instrow, 15, 0);
  }


void draw_moremisc(void)
  {
  int i;
  scrtxt_write_char(25, 2, hex_tab[NIB_HI(curr_octave)], 15, 0, 0);
  scrtxt_write_char(25+1, 2, hex_tab[NIB_LO(curr_octave)], 15, 0, 0);
  
  scrtxt_write_char(25, 4, hex_tab[NIB_HI(curr_pat)], 15, 0, 0);
  scrtxt_write_char(25+1, 4, hex_tab[NIB_LO(curr_pat)], 15, 0, 0);

  scrtxt_write_char(25, 6, hex_tab[NIB_HI(curr_row)], 15, 0, 0);
  scrtxt_write_char(25+1, 6, hex_tab[NIB_LO(curr_row)], 15, 0, 0);
  
  scrtxt_write_char(25, 8, hex_tab[NIB_HI(curr_speed)], 15, 0, 0);
  scrtxt_write_char(25+1, 8, hex_tab[NIB_LO(curr_speed)], 15, 0, 0);
  }



void draw_order(void)
  {
  int i, j, temp_order, OE_Col=OL_COL+3;

  temp_order=curr_order-5;
    temp_order=0;
    for(i=(OL_ROW-curr_order); ((i<(OL_ROW)) && (temp_order<order_tot)); i++, temp_order++)
      {
      if((i>2) && (i!=OL_ROW))
        {
        scrtxt_write_char(OL_COL, i, hex_tab[NIB_HI(temp_order)], OLPH_COLOR, OLBK_COLOR, 0);
        scrtxt_write_char(OL_COL+1, i, hex_tab[NIB_LO(temp_order)], OLPH_COLOR, OLBK_COLOR, 0);

        for(j=0; j<MAX_CHANNELS; j++) {
          scrtxt_write_char(OE_Col+3*j, i, hex_tab[NIB_HI(OrderList[temp_order][j])], OLPH_COLOR2, OLBK_COLOR, 0);
          scrtxt_write_char(OE_Col+3*j+1, i, hex_tab[NIB_LO(OrderList[temp_order][j])], OLPH_COLOR2, OLBK_COLOR, 0);
          }
        }
      }

    if(curr_order<order_tot)
      {
    if(edit_state==EDIT_ORDER)
      {
      scrtxt_write_char(OL_COL, OL_ROW, hex_tab[NIB_HI(curr_order)], 15, OLBK_COLOR, 0);
      scrtxt_write_char(OL_COL+1, OL_ROW, hex_tab[NIB_LO(curr_order)], 15, OLBK_COLOR, 0);
      for(j=0; j<MAX_CHANNELS; j++) {
        scrtxt_write_char(OE_Col+3*j, OL_ROW, hex_tab[NIB_HI(OrderList[temp_order][j])], 15, OLBK_COLOR, 0);
        scrtxt_write_char(OE_Col+3*j+1, OL_ROW, hex_tab[NIB_LO(OrderList[temp_order][j])], 15, OLBK_COLOR, 0);
        }
      }
    else
      {
      scrtxt_write_char(OL_COL, OL_ROW, hex_tab[NIB_HI(temp_order)], OLPH_COLOR, OLBK_COLOR, 0);
      scrtxt_write_char(OL_COL+1, OL_ROW, hex_tab[NIB_LO(temp_order)], OLPH_COLOR, OLBK_COLOR, 0);
      for(j=0; j<MAX_CHANNELS; j++) {
        scrtxt_write_char(OE_Col+3*j, OL_ROW, hex_tab[NIB_HI(OrderList[temp_order][j])], OLPH_COLOR2, OLBK_COLOR, 0);
        scrtxt_write_char(OE_Col+3*j+1, OL_ROW, hex_tab[NIB_LO(OrderList[temp_order][j])], OLPH_COLOR2, OLBK_COLOR, 0);
        }
      }
      }
  temp_order++;
  for(i=(OL_ROW+1); ((i<OL_ROW+5) && (temp_order<order_tot)); i++, temp_order++)
    {
    scrtxt_write_char(OL_COL, i, hex_tab[NIB_HI(temp_order)], OLPH_COLOR, OLBK_COLOR, 0);
    scrtxt_write_char(OL_COL+1, i, hex_tab[NIB_LO(temp_order)], OLPH_COLOR, OLBK_COLOR, 0);
    for(j=0; j<MAX_CHANNELS; j++) {
      scrtxt_write_char(OE_Col+3*j, i, hex_tab[NIB_HI(OrderList[temp_order][j])], OLPH_COLOR2, OLBK_COLOR, 0);
      scrtxt_write_char(OE_Col+3*j+1, i, hex_tab[NIB_LO(OrderList[temp_order][j])], OLPH_COLOR2, OLBK_COLOR, 0);
      }
    }
  DrawBgColLine_H(OL_COL, OL_COL+16, OL_ROW, 0x03, scrtxt_vidbuphph);
  DrawBgColLine_H(OL_COL+3+curr_chn*3, OL_COL+3+curr_chn*3+1, OL_ROW, 0x05, scrtxt_vidbuphph);
  DrawFgColLine_H(OL_COL+3+curr_chn*3, OL_COL+3+curr_chn*3+1, OL_ROW, 0x0F, scrtxt_vidbuphph);
  }


void    GetSnoteString(struct snote_struc *TheSnote, char *SnoteString, BYTE *SnoteColor, int Lit)
        {
        int     i;
        static  BYTE    FullColorTable[2][SNOTE_LENGTH_SCR]=
        {{NOTE_COLOR,NOTE_COLOR,OCT_COLOR,0,INST_COLOR,INST_COLOR,0,EFF_COLOR,EPARM_COLOR,EPARM_COLOR},
         {NOTE_LCOLOR,NOTE_LCOLOR,OCT_LCOLOR,0,INST_LCOLOR,INST_LCOLOR,0,EFF_LCOLOR,EPARM_LCOLOR,EPARM_LCOLOR}};
        BYTE    *ColorTab=FullColorTable[Lit];

        SnoteString[NOTE_POS]=SnoteString[NOTE_POS+1]=
        SnoteString[OCTAVE_POS]=SnoteString[INST1_POS]=
        SnoteString[INST2_POS]=SnoteString[EFF_POS]=
        SnoteString[EPARM1_POS]=SnoteString[EPARM2_POS]='-';
        SnoteString[3]=SnoteString[6]=0;

        for(i=0; i<SNOTE_LENGTH_SCR; i++) {
                if(Lit) SnoteColor[i]=EMPTY_LCOLOR;
                else    SnoteColor[i]=EMPTY_COLOR;
                }
          if(TheSnote->note) {
            if((*TheSnote).note==NOTE_OPHPH) {
              SnoteString[NOTE_POS]='O';
              SnoteString[NOTE_POS+1]='F';
              SnoteString[OCTAVE_POS]='F';
              SnoteColor[NOTE_POS]=ColorTab[NOTE_POS];
              SnoteColor[NOTE_POS+1]=ColorTab[NOTE_POS+1];
              SnoteColor[OCTAVE_POS]=ColorTab[OCTAVE_POS];
              }
            else {
              SnoteString[NOTE_POS]=note_char[TheSnote->note][NOTE_POS];
              SnoteString[NOTE_POS+1]=note_char[TheSnote->note][NOTE_POS+1];
              SnoteString[OCTAVE_POS]=hex_tab[TheSnote->octave];
              SnoteColor[NOTE_POS]=ColorTab[NOTE_POS];
              SnoteColor[NOTE_POS+1]=ColorTab[NOTE_POS+1];
              SnoteColor[OCTAVE_POS]=ColorTab[OCTAVE_POS];
              }
            }

          if((TheSnote->inst_num1) || (TheSnote->inst_num2)) {
            SnoteString[INST1_POS]=hex_tab[TheSnote->inst_num1];
            SnoteString[INST2_POS]=hex_tab[TheSnote->inst_num2];
            SnoteColor[INST1_POS]=ColorTab[INST1_POS];
            SnoteColor[INST2_POS]=ColorTab[INST2_POS];
            }
          if(TheSnote->ephphect) {
            SnoteString[EFF_POS]=hex_tab[TheSnote->ephphect];
            SnoteString[EPARM1_POS]=hex_tab[TheSnote->e_parm1];
            SnoteString[EPARM2_POS]=hex_tab[TheSnote->e_parm2];

            SnoteColor[EFF_POS]=ColorTab[EFF_POS];
            SnoteColor[EPARM1_POS]=ColorTab[EPARM1_POS];
            SnoteColor[EPARM2_POS]=ColorTab[EPARM2_POS];
            }
        }

/*void    GetSnoteDPCMString(SnoteDPCM *TheSnote, char *SnoteString, BYTE *SnoteColor, int Lit)
        {
        int     i;
        static  BYTE    FullColorTable[2][SNOTE_LENGTH_SCR]=
        {{NOTE_COLOR,NOTE_COLOR,OCT_COLOR,0,INST_COLOR,INST_COLOR,0,EFF_COLOR,EPARM_COLOR,EPARM_COLOR},
         {NOTE_LCOLOR,NOTE_LCOLOR,OCT_LCOLOR,0,INST_LCOLOR,INST_LCOLOR,0,EFF_LCOLOR,EPARM_LCOLOR,EPARM_LCOLOR}};
        BYTE    *ColorTab=FullColorTable[Lit];

        SnoteString[NOTE_POS]=SnoteString[NOTE_POS+1]=
        SnoteString[OCTAVE_POS]=SnoteString[INST1_POS]=
        SnoteString[INST2_POS]=SnoteString[EFF_POS]=
        SnoteString[EPARM1_POS]=SnoteString[EPARM2_POS]='-';
        SnoteString[3]=SnoteString[6]=0;

        for(i=0; i<SNOTE_LENGTH_SCR; i++) {
                if(Lit) SnoteColor[i]=EMPTY_LCOLOR;
                else    SnoteColor[i]=EMPTY_COLOR;
                }
          if(TheSnote->Note) {
            SnoteString[NOTE_POS]=note_char[TheSnote->note][NOTE_POS];
            SnoteString[NOTE_POS+1]=note_char[TheSnote->note][NOTE_POS+1];
            SnoteString[OCTAVE_POS]=hex_tab[TheSnote->octave];
            SnoteString[INST1_POS]=hex_tab[TheSnote->inst_num1];
            SnoteString[INST2_POS]=hex_tab[TheSnote->inst_num2];

            SnoteColor[NOTE_POS]=ColorTab[NOTE_POS];
            SnoteColor[NOTE_POS+1]=ColorTab[NOTE_POS+1];
            SnoteColor[OCTAVE_POS]=ColorTab[OCTAVE_POS];
            SnoteColor[INST1_POS]=ColorTab[INST1_POS];
            SnoteColor[INST2_POS]=ColorTab[INST2_POS];
            }
          if(TheSnote->ephphect) {
            SnoteString[EFF_POS]=hex_tab[TheSnote->ephphect];
            SnoteString[EPARM1_POS]=hex_tab[TheSnote->e_parm1];
            SnoteString[EPARM2_POS]=hex_tab[TheSnote->e_parm2];

            SnoteColor[EFF_POS]=ColorTab[EFF_POS];
            SnoteColor[EPARM1_POS]=ColorTab[EPARM1_POS];
            SnoteColor[EPARM2_POS]=ColorTab[EPARM2_POS];
            }
        }*/


DWORD   MakeSampleChunk(void *SampleChunkData, InfoHeader *Info);
DWORD   PackPattern20(ChannelPattern *NT2_Pattern, BYTE *PackedData, int Chn);
DWORD   PackPatternDPCM20(ChannelPattern *NT2_Pattern, BYTE *PackedData);
DWORD   UnPackPattern20(BYTE *PackedData, ChannelPattern *NT2_Pattern, int Chn);
DWORD   UnPackPatternDPCM20(BYTE *PackedData, ChannelPattern *NT2_Pattern);
DWORD   PackInstrument20(NedInstStruc *Src, NedInstStruc *Dest);
DWORD   PackInstrumentDPCM20(InstDPCM *TheInst, BYTE *Dest);
DWORD   UnPackInstrument20(NedInstStrucNew *Src, struct inst_struc *Dest);
DWORD   UnPackInstrument10(struct ned_inst_struc *Src, struct inst_struc *Dest);
DWORD   UnPackInstrumentDPCM20(BYTE *Src, InstDPCM *TheInst);
DWORD   UnPackOrderEntry20(BYTE *Src, BYTE *Dest);




int     SaveNedInfo20(char *FileName, DWORD *FileOffset, InfoHeader *Info)
        {
        void    *SampleChunkData=(BYTE *)malloc(512*1024);
        long    SampleChunkSize;

        strcpy(Info->ID, "NED: ");
        memset(Info->ModuleName, 0, sizeof(Info->ModuleName));
        strcpy(Info->ModuleName, "Noname");
        Info->Terminator=0x1A;
        Info->HeaderSize=sizeof(InfoHeader);
        SampleChunkSize=MakeSampleChunk(SampleChunkData, Info);
        Info->Flags=0;

        // !!
        //Info->Version=0x20;

        Info->Version=0x21;

        *FileOffset=writefile(FileName, *FileOffset, sizeof(InfoHeader), Info);
        *FileOffset=AppendFile(FileName, *FileOffset, SampleChunkSize, SampleChunkData);
        free(SampleChunkData);
        return(1);
        }

DWORD   MakeSampleChunk(void *SampleChunkData, InfoHeader *Info)
        {
        int             i, j;
        BYTE            *BytePtr=(BYTE *)SampleChunkData,
                        *OldBytePtr=(BYTE *)SampleChunkData;
        SampleInstInfo  *InstPtr=(SampleInstInfo *)BytePtr;
        int             NumInstruments=0;
        DWORD           TotalSampleSize=0;
        DWORD           ChunkSize=0;
        SampInstInfo    *InstP=(SampInstInfo *)SampleChunkData;

    
        for(i=0; i<MAX_INSTRUMENTS_DPCM; i++) {
          for(j=0; j<MAX_DPCM_SAMPLES; j++) {
            if((SampleInst[i].Sample[j].SamplePtr) || (strlen(InstName[i])) ||
              (strlen(SampleName[i][j]))) NumInstruments=i+1;
            }
          }

        Info->NumSampleInstrumentsNTSC=NumInstruments;
        Info->NumSampleInstrumentsPAL=0;

        BytePtr+=NumInstruments*sizeof(SampInstInfo);

          // Fill in instrument info
        for(i=0; i<NumInstruments; i++) {
          memcpy(InstP[i].NoteTable, NoteTable[i], MAX_DPCM_OCTAVES*12);
          memset(InstP[i].Name, 0, DPCM_INAME_LENGTH);
          if(strlen(InstName[i])) memcpy(InstP[i].Name, InstName[i], DPCM_INAME_LENGTH);

          InstP[i].NumSamples=8;

          for(j=0; j<MAX_DPCM_SAMPLES; j++) {
            memset(InstP[i].Sample[j].Name, 0, DPCM_SNAME_LENGTH);
            if(strlen(SampleName[i][j]))
              memcpy(InstP[i].Sample[j].Name, SampleName[i][j], DPCM_SNAME_LENGTH);
            if(SampleInst[i].Sample[j].SamplePtr) {
                InstP[i].Sample[j].Offset=TotalSampleSize;
                memcpy(&BytePtr[TotalSampleSize], SampleInst[i].Sample[j].SamplePtr, DPCMLength2Normal(SampleInst[i].Sample[j].Length));
                TotalSampleSize+=DPCMLength2Normal(SampleInst[i].Sample[j].Length);
                //printf("Length: %d, TotalSampleSize: %d\n\b\b\b",(int)SampleInst[i].Sample[j].Length, TotalSampleSize);
                }
            else InstP[i].Sample[j].Offset=0xFFFFFFFF;
            InstP[i].Sample[j].InitAmp=SampleInst[i].Sample[j].Volume;
            InstP[i].Sample[j].LoopAmp=0x40;
            InstP[i].Sample[j].Length=SampleInst[i].Sample[j].Length;
            InstP[i].Sample[j].LoopStart=SampleInst[i].Sample[j].LoopStart;
            InstP[i].Sample[j].LoopFlag=SampleInst[i].Sample[j].LoopFlag;
            }
          }
        ChunkSize=sizeof(SampInstInfo)*NumInstruments+TotalSampleSize;

        Info->SampleDataSizeNTSC=TotalSampleSize;
        Info->SampleDataSizePAL=0;

        return(ChunkSize);
        }


int     SaveNedData20(char *FileName, DWORD FileOffset, InfoHeader *Info)
        {
        DataHeader      Data;
        int     TotalPatterns[MAX_CHANNELS]={1,1,1,1,1}, EmptyFlag,
                TotalInstruments=0,
                TotalInstrumentsDPCM=0,
                OrderLength=1;
        int     i, j, k;
        DWORD   TempOffset=0;
        BYTE    *PackedPatternData[MAX_CHANNELS];
        DWORD   TotalPackedPatternSize[MAX_CHANNELS];
        WORD    PackedPatternSize[MAX_CHANNELS][MAX_PATTERNS];
        WORD    PackedPatternOffset[MAX_CHANNELS][MAX_PATTERNS];
        //BYTE    PackedOrderList[NT_MAX_ORDER][3];
        BYTE    PackedOrderList[NT_MAX_ORDER][4];
        DWORD   OldFileOffset;
        BYTE    PackedInstDPCM[15][17];
        //NedInstStruc    NedInst[16];
        NedInstStrucNew    NedInst[16];

        FILE    *fp;

        fp=fopen("TEMP.REP","w");

        for(i=0; i<MAX_CHANNELS; i++) {
          TotalPackedPatternSize[i]=0;
          }

        // Find out how many patterns there are
        for(i=0; i<MAX_A_CHANNELS; i++)
          for(j=0; j<MAX_PATTERNS; j++)
            if(!PatternIsEmpty(&ChnPattern[i][j])) TotalPatterns[i]=j+1;


          for(j=0; j<MAX_PATTERNS_DPCM; j++)
            if(!PatternIsEmpty(&ChnPattern[DPCM_CHN][j])) TotalPatterns[DPCM_CHN]=j+1;


        // Need to check order list too...
        for(i=0; i<MAX_CHANNELS; i++)
          for(j=0; j<NT_MAX_ORDER; j++) {
            if(OrderList[j][i]>=TotalPatterns[i])
              TotalPatterns[i]=OrderList[j][i]+1;
            }
            


        /*for(i=0; i<MAX_INSTRUMENTS; i++) {
          if(inst[i].duty_cycle || inst[i].hold_note ||
             inst[i].envelope_phixed || inst[i].volume ||
             inst[i].variable_phreq || inst[i].phq_changespd || inst[i].ishi2lo ||
             inst[i].phq_range || inst[i].active_tlength) TotalInstruments=i+1;
             }*/
        TotalInstruments=16;

        for(i=0; i<MAX_INSTRUMENTS_DPCM; i++)
          for(j=0; j<MAX_DPCM_SAMPLES; j++)
            if(SampleInst[i].Sample[j].Length) TotalInstrumentsDPCM=i+1;


        for(i=0; i<MAX_CHANNELS; i++)
          PackedPatternData[i]=(BYTE *)malloc(TotalPatterns[i]*64*4);

        for(i=0; i<TotalInstruments; i++) {
          /*NedInst[i].ArpXY=(inst[i].ArpX | (inst[i].ArpY<<4));
          NedInst[i].ArpZAndCR0=(inst[i].ArpZ | (((inst[i].duty_cycle<<2) | (inst[i].hold_note<<1) | inst[i].envelope_phixed)<<4));
          NedInst[i].AutoVibrato=((inst[i].VibratoSpeed<<4) | inst[i].VibratoDepth);
          NedInst[i].AutoTremolo=((inst[i].TremoloSpeed<<4) | inst[i].TremoloDepth);
          NedInst[i].VolumeAndVolSlide=((inst[i].volume<<4) | inst[i].VolumeSlide);
          NedInst[i].TimeLengthAndVSlideDir=(inst[i].active_tlength | (inst[i].VolumeSlideDir<<7) | (inst[i].LoopedNoise<<6));
          NedInst[i].Porta=((inst[i].PortaDir<<7) | (inst[i].Porta & 0x7F));
          NedInst[i].CR1=(inst[i].variable_phreq<<7)+(inst[i].phq_changespd<<4)+
                         (inst[i].ishi2lo<<3)+(inst[i].phq_range);*/

          NedInst[i].CR0=((inst[i].duty_cycle<<2) | (inst[i].hold_note<<1) | 0x01)<<4;
          if(inst[i].PortaDir==0)
            NedInst[i].Porta=inst[i].Porta & 0x7F;
          else
            NedInst[i].Porta=-(inst[i].Porta & 0x7F);
          NedInst[i].AutoVibrato=((inst[i].VibratoSpeed<<4) | inst[i].VibratoDepth);
          NedInst[i].AutoTremolo=((inst[i].TremoloSpeed<<4) | inst[i].TremoloDepth);
          NedInst[i].TimeLengthAndVSlideDir=(inst[i].active_tlength | (inst[i].VolumeSlideDir<<7));
          NedInst[i].VolumeAndVolSlide=((inst[i].volume<<4) | inst[i].VolumeSlide);

          NedInst[i].ArpXY=(inst[i].ArpX | (inst[i].ArpY<<4));
          NedInst[i].ArpZAndMisc=(inst[i].ArpZ | ((inst[i].ReversedArpeggio & 1)<<7) | ((inst[i].LoopedNoise & 1)<<6) | ((inst[i].NonLoopedArpeggio & 1)<<5));
          }
        
        for(i=0; i<NT_MAX_ORDER; i++)
          for(j=0; j<MAX_CHANNELS; j++)
            if(OrderList[i][j]) OrderLength=i+1;
        
        for(i=0; i<OrderLength; i++) {
            DWORD PackedOrderEntry;
            PackedOrderEntry=OrderList[i][SQRWAV1_CHN]+
                            (OrderList[i][SQRWAV2_CHN]<<5)+
                            (OrderList[i][TRIWAV_CHN]<<10)+
                            (OrderList[i][NOSWAV_CHN]<<15)+
                            (OrderList[i][DPCM_CHN]<<20);
            PackedOrderList[i][0]=(PackedOrderEntry & 0xFF);
            PackedOrderList[i][1]=((PackedOrderEntry>>8) & 0xFF);
            PackedOrderList[i][2]=((PackedOrderEntry>>16) & 0xFF);
            PackedOrderList[i][3]=((PackedOrderEntry>>24) & 0xFF);
            }

          // First pack synth channel patterns
        for(i=0; i<MAX_CHANNELS; i++) {
          fprintf(fp,"\nChannel %d :\n", i);
          for(j=0; j<TotalPatterns[i]; j++) {
            PackedPatternSize[i][j]=PackPattern20(&ChnPattern[i][j], &PackedPatternData[i][TotalPackedPatternSize[i]], i);
            TotalPackedPatternSize[i]+=PackedPatternSize[i][j];
            fprintf(fp,"\nPattern %d: %d",j,PackedPatternSize[i][j]);
            }
          fprintf(fp,"\n\nTotal Size of channel %d 's patterns: %d\n",i,TotalPackedPatternSize[i]);
          }
        
        /*  // Then pack DPCM patterns
          for(j=0; j<TotalPatterns[DPCM_CHN]; j++) {
            PackedPatternSize[DPCM_CHN][j]=PackPatternDPCM20(&ChnPattern[DPCM_CHN][j], &PackedPatternData[DPCM_CHN][TotalPackedPatternSize[DPCM_CHN]]);
            TotalPackedPatternSize[DPCM_CHN]+=PackedPatternSize[DPCM_CHN][j];
            }*/


        Data.Version=0x20;
        Data.HeaderSize=sizeof(DataHeader);
        Data.RestartPosition=RestartPosition;
        Data.SongLength=OrderLength;
        Data.NumPatterns[SQRWAV1_CHN]=TotalPatterns[SQRWAV1_CHN];
        Data.NumPatterns[SQRWAV2_CHN]=TotalPatterns[SQRWAV2_CHN];
        Data.NumPatterns[TRIWAV_CHN]=TotalPatterns[TRIWAV_CHN];
        Data.NumPatterns[NOSWAV_CHN]=TotalPatterns[NOSWAV_CHN];
        Data.NumPatternsDPCM=TotalPatterns[DPCM_CHN];
        Data.NumInstruments=TotalInstruments;

        Data.NumInstrumentsDPCM=TotalInstrumentsDPCM;
        Data.NumInstrumentsDPCM=0;

        Data.Flags=0;
        Data.InitTempo=curr_speed;

        FileOffset=AppendFile(FileName,FileOffset,Data.HeaderSize,&Data);
        if(Data.NumInstruments)
//          FileOffset=AppendFile(FileName,FileOffset,TotalInstruments*sizeof(NedInstStruc),NedInst);
          FileOffset=AppendFile(FileName,FileOffset,TotalInstruments*sizeof(NedInstStrucNew),NedInst);

        if(Data.NumInstrumentsDPCM) {
          for(i=0; i<TotalInstrumentsDPCM; i++)
                  PackInstrumentDPCM20(&SampleInst[i], PackedInstDPCM[i]);
          FileOffset=AppendFile(FileName,FileOffset,TotalInstrumentsDPCM*17,PackedInstDPCM);
          }

        
//        FileOffset=AppendFile(FileName,FileOffset,(OrderLength*3),PackedOrderList);
        FileOffset=AppendFile(FileName,FileOffset,(OrderLength*4),PackedOrderList);

        // !!
        TempOffset=FileOffset+(DWORD)((TotalPatterns[0]+TotalPatterns[1]+
          TotalPatterns[2]+TotalPatterns[3]+TotalPatterns[DPCM_CHN])*sizeof(WORD))-((DWORD)Info->HeaderSize+Info->NumSampleInstrumentsNTSC*sizeof(SampInstInfo)+Info->SampleDataSizeNTSC);

        for(i=0; i<MAX_CHANNELS; i++) {
          for(j=0; j<TotalPatterns[i]; j++) {
            if(PackedPatternSize[i][j]) {
              PackedPatternOffset[i][j]=TempOffset;
              // PackedPatternOffset[i][j]|=0x8000;
              }
            else PackedPatternOffset[i][j]=0;
            TempOffset+=PackedPatternSize[i][j];
            }
        FileOffset=AppendFile(FileName,FileOffset,TotalPatterns[i]*sizeof(WORD),PackedPatternOffset[i]);
        }

        
        for(i=0; i<MAX_CHANNELS; i++) {
          FileOffset=AppendFile(FileName,FileOffset,TotalPackedPatternSize[i],PackedPatternData[i]);
          free(PackedPatternData[i]);
          }
        
        fclose(fp);

        //return(1);
        
        return(1);
        }



int     SaveNed20(char *FileName)
        {
        InfoHeader Info;
        DWORD   FileOffset=0;
        SaveNedInfo20(FileName, &FileOffset, &Info);
        return(SaveNedData20(FileName, FileOffset, &Info));
        return(1);
        }


       

int     LoadNed(char *FileName)
        {
        int     FileOk=1;
        int     i;
        InfoHeader      Info;
        DataHeader      Data;
        char    ID_Check[5]={'N','E','D',':',' '};
        char    ID_Check10[4]={'N','E','D',0x10};
        readfile(FileName, 0, sizeof(InfoHeader), &Info);

        if((Info.Version==0x20) || (Info.Version==0x21))
          readfile(FileName, Info.HeaderSize+Info.NumSampleInstrumentsNTSC*sizeof(SampInstInfo)+Info.SampleDataSizeNTSC, sizeof(DataHeader), &Data);
        else
          readfile(FileName, Info.HeaderSize, sizeof(DataHeader), &Data);
                // Test if header is ok...
        for(i=0; i<5; i++)
          if(Info.ID[i]!=ID_Check[i]) FileOk=0;
        if(FileOk) {

          if(Data.Version==0x20) {
                ResetEditors();
                return(LoadNed20(FileName));}
          return(0);
          }
                // If not, test if it's a version 1.0 NED
        FileOk=1;
        for(i=0; i<4; i++)
          if(Info.ID[i]!=ID_Check10[i]) FileOk=0;
        if(FileOk) {
                ResetEditors();
                return(LoadNed10(FileName));}
        return(0);
        }



int     LoadNed10(char *FileName)
        {
        struct ned_header_struc NedHeader10;
        int     TotalPatterns=1, EmptyFlag,
                TotalInstruments=0,
                OrderLength=1;
        int     i, j, k;
        DWORD   FileOffset=0;
        DWORD   TempOffset=0;
        BYTE    *PackedPatternData=NULL,
                TotalPackedPatternSize=0;
        WORD    PackedPatternSize[MAX_PATTERNS];
        WORD    PackedPatternOffset[MAX_PATTERNS];
        BYTE    PackedPattern[64*4*3];
        BYTE    OrderList10[128];

        FileOffset=readfile(FileName,FileOffset,sizeof(NedHeader10),&NedHeader10);
        

                // Load and unpack instruments
        FileOffset=readfile(FileName,FileOffset,sizeof(struct ned_inst_struc)*NedHeader10.inst_tot,ned_inst);
        for(i=0; i<NedHeader10.inst_tot; i++)
          UnPackInstrument10(&ned_inst[i], &inst[i]);

        FileOffset=readfile(FileName,FileOffset,NedHeader10.order_tot,OrderList10);
        FileOffset=readfile(FileName,FileOffset,NedHeader10.pat_tot*sizeof(WORD),PackedPatternOffset);

        for(i=0; i<NedHeader10.pat_tot; i++) {
          readfile(FileName,PackedPatternOffset[i],64*4*3,PackedPattern);
          ConvertNed10Pat2ChnPat(PackedPattern,&ChnPattern[0][i],&ChnPattern[1][i],
                                 &ChnPattern[2][i],&ChnPattern[3][i]);
          for(j=0; j<NedHeader10.order_tot; j++)
            if(OrderList10[j]==i) {
                OrderList[j][0]=i;
                OrderList[j][1]=i;
                OrderList[j][2]=i;
                OrderList[j][3]=i;
                OrderList[j][4]=0;
                }
          }
        order_tot=NedHeader10.order_tot;
        RestartPosition=0;
        ClearOrder2Max(order_tot);
        return(1);
        }

void    ConvertNed10Pat2ChnPat(BYTE *Ned10Pat, ChannelPattern *P_A, ChannelPattern *P_B, ChannelPattern *P_C, ChannelPattern *P_D)
        {
        int     i, j;
        ChannelPattern  *CPat[4];
        DWORD   PackedSnote;
        int     Note, Oct, Inst, Eff, EffParm;
        BYTE    sn1, sn2, sn3;
        int     TempNote, TempOct;

        CPat[0]=P_A;
        CPat[1]=P_B;
        CPat[2]=P_C;
        CPat[3]=P_D;

        for(i=0; i<64; i++)
          for(j=0; j<MAX_A_CHANNELS; j++) {
            sn1=Ned10Pat[(4*3)*i+3*j];
            sn2=Ned10Pat[(4*3)*i+3*j+1];
            sn3=Ned10Pat[(4*3)*i+3*j+2];

            if(!sn1) CPat[j]->Row[i].note=0;
            else
                {
                sn1--;
                TempOct=(int)(sn1/12);
                TempNote=(int)(sn1-(TempOct*12));
                if(TempNote==11) {
                  TempNote=BuggyNote2Normal[TempNote%12];
                  TempOct++;
                  }
                else
                  TempNote=BuggyNote2Normal[TempNote%12];
                TempNote++;
                TempOct++;
                CPat[j]->Row[i].note=TempNote;
                CPat[j]->Row[i].octave=TempOct;
            CPat[j]->Row[i].inst_num1=NIB_HI(sn2 & 0x1F);
            CPat[j]->Row[i].inst_num2=NIB_LO(sn2 & 0x1F);;
                }
            CPat[j]->Row[i].ephphect=nedeph2eph[(sn2 & 0xE0)>>5];
            CPat[j]->Row[i].e_parm1=NIB_HI(sn3);
            CPat[j]->Row[i].e_parm2=NIB_LO(sn3);
            if(CPat[j]->Row[i].ephphect==7)
              CPat[j]->Row[i].ephphect=8;
            if(CPat[j]->Row[i].ephphect==0xC) {
              if(CPat[j]->Row[i].e_parm1==0) {
                int     Parm=CPat[j]->Row[i].e_parm2<<2;
                CPat[j]->Row[i].e_parm1=(Parm>>4) & 0xF;
                CPat[j]->Row[i].e_parm2=(Parm & 0xF);
                }
              else
                if(CPat[j]->Row[i].e_parm1==1) {
                  CPat[j]->Row[i].ephphect=0xA;
                  CPat[j]->Row[i].e_parm1=CPat[j]->Row[i].e_parm2;
                  CPat[j]->Row[i].e_parm2=0;
                  }
              else
                if(CPat[j]->Row[i].e_parm1==2) {
                  CPat[j]->Row[i].ephphect=0xA;
                  CPat[j]->Row[i].e_parm1=0;
                  }
              }
            }
        }


void    ReadSampleChunk(void *SampleChunkData, InfoHeader *Info)
        {
        int             i, j;
        BYTE            *BytePtr=(BYTE *)SampleChunkData,
                        *OldBytePtr=(BYTE *)SampleChunkData;
        SampleInstInfo  *InstPtr=(SampleInstInfo *)BytePtr;
        int             NumInstruments;
        DWORD           TotalSampleSize;
        DWORD           ChunkSize=0;
        DWORD           SampleSize[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES+1];
        SampInstInfo    *InstP=(SampInstInfo *)SampleChunkData;

        TotalSampleSize=(*Info).SampleDataSizeNTSC;
        NumInstruments=(*Info).NumSampleInstrumentsNTSC;
        BytePtr+=NumInstruments*sizeof(SampInstInfo);
        
        for(i=0; i<NumInstruments; i++) 
          for(j=0; j<8; j++) 
            SampleSize[i*MAX_DPCM_SAMPLES+j]=InstP[i].Sample[j].Offset;
        SampleSize[NumInstruments*MAX_DPCM_SAMPLES]=TotalSampleSize;

        for(i=0; i<(NumInstruments*MAX_DPCM_SAMPLES); i++) {
          if(SampleSize[i]==0xFFFFFFFF)
            SampleSize[i]=0;
          else {
            j=1;
            while(SampleSize[i+j]==0xFFFFFFFF) j++;
            SampleSize[i]=SampleSize[i+j]-SampleSize[i];
            }
          }
       
          // Fill in instrument info
        for(i=0; i<NumInstruments; i++) {
          memcpy(NoteTable[i], InstP[i].NoteTable, MAX_DPCM_OCTAVES*12);
          if(strlen(InstP[i].Name))
            memcpy(InstName[i], InstP[i].Name, DPCM_INAME_LENGTH);
          for(j=0; j<MAX_DPCM_SAMPLES; j++) {
            if(strlen(InstP[i].Sample[j].Name))
              memcpy(SampleName[i][j],InstP[i].Sample[j].Name,DPCM_SNAME_LENGTH);
            if(InstP[i].Sample[j].Offset!=0xFFFFFFFF) {
                if(SampleInst[i].Sample[j].SamplePtr) free(SampleInst[i].Sample[j].SamplePtr);
                SampleInst[i].Sample[j].SamplePtr=(BYTE *)malloc(SampleSize[i*MAX_DPCM_SAMPLES+j]);
                memcpy(SampleInst[i].Sample[j].SamplePtr, &BytePtr[InstP[i].Sample[j].Offset], SampleSize[i*MAX_DPCM_SAMPLES+j]);
                }
            SampleInst[i].Sample[j].Length=InstP[i].Sample[j].Length;
            SampleInst[i].Sample[j].LoopStart=InstP[i].Sample[j].LoopStart;
            SampleInst[i].Sample[j].LoopFlag=InstP[i].Sample[j].LoopFlag;
            SampleInst[i].Sample[j].Volume=InstP[i].Sample[j].InitAmp;
            SampleInst[i].Sample[j].LoopAmp=InstP[i].Sample[j].LoopAmp;
            }
          }
        }


int     LoadNed20(char *FileName)
        {
        InfoHeader      Info;
        DataHeader      Data;
        int     TotalPatterns=1, EmptyFlag,
                TotalInstruments=0,
                OrderLength=1;
        int     i, j, k;
        DWORD   FileOffset=0;
        DWORD   TempOffset=0;
        DWORD   TotalPackedPatternSize=0;
        WORD    PackedPatternSize[MAX_PATTERNS];
        WORD    PackedPatternOffset[MAX_CHANNELS][MAX_PATTERNS];
//        BYTE    PackedOrderList[NT_MAX_ORDER][3];
        BYTE    PackedOrderList[NT_MAX_ORDER][4];
        char    ID_Check[5]={'N','E','D',':',' '};
        BYTE    PackedPattern[256];
        BYTE    PackedInstDPCM[15][17];
        BYTE    *SampleChunk=NULL;
        //NedInstStruc    NedInst[16];
        NedInstStrucNew   NedInst[16];

        //readfile(FileName, 0, sizeof(InfoHeader), &Info);
        
        FileOffset=readfile(FileName,FileOffset,sizeof(InfoHeader),&Info);
        FileOffset=Info.HeaderSize;

        // MAY BE UNNECESSARY
        for(i=0; i<MAX_INSTRUMENTS_DPCM; i++)
          for(j=0; j<MAX_DPCM_SAMPLES; j++)
                if(SampleInst[i].Sample[j].SamplePtr) free(SampleInst[i].Sample[j].SamplePtr);

        SampleChunk=(BYTE *)malloc(Info.NumSampleInstrumentsNTSC*sizeof(SampInstInfo)+Info.SampleDataSizeNTSC);

        if((Info.HeaderSize==sizeof(InfoHeader)) && ((Info.Version==0x20) || (Info.Version==0x21))) {
          if(Info.NumSampleInstrumentsNTSC || Info.SampleDataSizeNTSC) {
            FileOffset=readfile(FileName,Info.HeaderSize,Info.NumSampleInstrumentsNTSC*sizeof(SampInstInfo)+Info.SampleDataSizeNTSC,SampleChunk);
            ReadSampleChunk(SampleChunk, &Info);
            printf("HRRRM!!\n");
            }
          }


        //FileOffset=readfile(FileName,FileOffset,sizeof(DataHeader),&Data);
        readfile(FileName,FileOffset,sizeof(DataHeader),&Data);
        FileOffset+=Data.HeaderSize;

                         // Load and unpack instruments
        if(Data.NumInstruments) {
//          FileOffset=readfile(FileName,FileOffset,sizeof(NedInstStruc)*Data.NumInstruments, NedInst);
          FileOffset=readfile(FileName,FileOffset,sizeof(NedInstStrucNew)*Data.NumInstruments, NedInst);
          for(i=0; i<Data.NumInstruments; i++)
                UnPackInstrument20(&NedInst[i], &inst[i]);
          }

                        // Load and unpack DPCM instruments
        if(Data.NumInstrumentsDPCM) {
          FileOffset=readfile(FileName,FileOffset,Data.NumInstrumentsDPCM*17,PackedInstDPCM);
          for(i=0; i<Data.NumInstrumentsDPCM; i++)
                UnPackInstrumentDPCM20(PackedInstDPCM[i], &SampleInst[i]);
          }

        free(SampleChunk);

               // Load and unpack order list
//        FileOffset=readfile(FileName,FileOffset,3*(Data.SongLength),PackedOrderList);
        FileOffset=readfile(FileName,FileOffset,4*(Data.SongLength),PackedOrderList);
        for(i=0; i<(Data.SongLength); i++)
                UnPackOrderEntry20(PackedOrderList[i], OrderList[i]);

                    // Load and unpack patterns
        for(i=0; i<MAX_CHANNELS; i++) {
          int TempoLino;
          if(i==DPCM_CHN)
            TempoLino=Data.NumPatterns[i];
          else
            TempoLino=Data.NumPatternsDPCM;
          //if(Data.NumPatterns[i]) {
          if(TempoLino) {
            FileOffset=readfile(FileName,FileOffset,sizeof(WORD)*Data.NumPatterns[i],PackedPatternOffset[i]);
            for(j=0; j<Data.NumPatterns[i]; j++) {
                  if(PackedPatternOffset[i][j]) {
                    if(Info.Version==0x21)
                        readfile(FileName,(PackedPatternOffset[i][j] & 0x7FFF)+Info.HeaderSize+Info.NumSampleInstrumentsNTSC*sizeof(SampInstInfo)+Info.SampleDataSizeNTSC, 256, PackedPattern);
                    else
                        readfile(FileName,(PackedPatternOffset[i][j] & 0x7FFF)+Info.HeaderSize, 256, PackedPattern);
                    // !!
                    UnPackPattern20(PackedPattern, &ChnPattern[i][j], i);
                    }
                  else ClearPattern(&ChnPattern[i][j]);
                  }
            }
          }
                    // Load and unpack DPCM patterns
          /* if(Data.NumPatternsDPCM) {
            FileOffset=readfile(FileName,FileOffset,sizeof(WORD)*Data.NumPatternsDPCM,PackedPatternOffset[DPCM_CHN]);
            for(j=0; j<Data.NumPatternsDPCM; j++) {
                  if(PackedPatternOffset[DPCM_CHN][j]) {
                    readfile(FileName,(PackedPatternOffset[DPCM_CHN][j] & 0x7FFF)+Info.HeaderSize, 64*3, PackedPattern);
                    UnPackPatternDPCM20(PackedPattern, &ChnPattern[DPCM_CHN][j]);
                    }
                  else ClearPattern(&ChnPattern[DPCM_CHN][j]);
                  }
            } */
        

        order_tot=Data.SongLength;
        ClearOrder2Max(order_tot);
        curr_order=0;
        RestartPosition=Data.RestartPosition;
        return(1);
        }

DWORD   PackPattern20(ChannelPattern *NT2_Pattern, BYTE *PackedData, int Chn)
        {
        int     Note, Oct, Inst, Eff, EffParm, Tone;
        int     i, j;
        DWORD   PackedSize=0;
        DWORD   PackedSnote;
        int     TotalRows=0,
                TotalBitEntries;
        int     NibblesUsed=0;
        int     StartOfData;

        if(PatternIsEmpty(NT2_Pattern)) return(0);


        for(i=0; i<64; i++) {
          struct snote_struc Snote=(*NT2_Pattern).Row[i];
          if(Snote.note || Snote.octave || Snote.inst_num1 || Snote.inst_num2 ||
             Snote.ephphect || Snote.e_parm1 || Snote.e_parm2)
            TotalRows=i+1;
          }
        
        //TotalRows=64; // Remove!
        PackedData[0]=0;
        PackedData[1]=TotalRows-1;

        TotalBitEntries=(TotalRows+1)>>1;

        //TotalBitEntries=32; // Remove!

        memset((PackedData+2), 0, TotalBitEntries+64*3);
        for(i=0; i<TotalRows; i++) {
          int HalfBitEntry=0;
          Note=NT2_Pattern->Row[i].note;
          Oct=NT2_Pattern->Row[i].octave;
          Inst=(NT2_Pattern->Row[i].inst_num1<<4)+NT2_Pattern->Row[i].inst_num2;
          Eff=NT2_Pattern->Row[i].ephphect;
          EffParm=(NT2_Pattern->Row[i].e_parm1<<4)+NT2_Pattern->Row[i].e_parm2;

          if(!Note) Tone=0;
          else {
            if(Note==NOTE_OPHPH)
              Tone=97;
            else
              Tone=Oct*12+Note;
            }

          if(Tone) {
            HalfBitEntry|=1;

            if(Chn==NOSWAV_CHN) {
              PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(15-((Tone-1) & 0xF))<<((NibblesUsed & 1)*4);
              NibblesUsed++;
              }
            else {
              if((NibblesUsed & 1)==0) {
                PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(Tone & 0xF)<<((NibblesUsed & 1)*4);
                //PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((Tone>>4) & 0xF)<<((NibblesUsed & 1)*4);
                NibblesUsed++;
                PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((Tone>>4) & 0xF)<<((NibblesUsed & 1)*4);
                //PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(Tone & 0xF)<<((NibblesUsed & 1)*4);
                NibblesUsed++;
                }
              else {
                PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((Tone>>4) & 0xF)<<((NibblesUsed & 1)*4);
                NibblesUsed++;
                PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(Tone & 0xF)<<((NibblesUsed & 1)*4);
                NibblesUsed++;
                }
              }
            }
          if(Inst) {
            HalfBitEntry|=2;
            PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((Inst-1) & 0xF)<<((NibblesUsed & 1)*4);
            NibblesUsed++;}
          if(Eff) {
            HalfBitEntry|=4;
            PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(Eff & 0xF)<<((NibblesUsed & 1)*4);
            NibblesUsed++;
            }
          if(EffParm) {
            HalfBitEntry|=8;
            if((NibblesUsed & 1)==0) {
              PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(EffParm & 0xF)<<((NibblesUsed & 1)*4);
              //PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((EffParm>>4) & 0xF)<<((NibblesUsed & 1)*4);
              NibblesUsed++;
              PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((EffParm>>4) & 0xF)<<((NibblesUsed & 1)*4);
              //PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(EffParm & 0xF)<<((NibblesUsed & 1)*4);
              NibblesUsed++;
              }
            else {
              //PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(EffParm & 0xF)<<((NibblesUsed & 1)*4);
              PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((EffParm>>4) & 0xF)<<((NibblesUsed & 1)*4);
              NibblesUsed++;
              //PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=((EffParm>>4) & 0xF)<<((NibblesUsed & 1)*4);
              PackedData[2+TotalBitEntries+(NibblesUsed>>1)]|=(EffParm & 0xF)<<((NibblesUsed & 1)*4);
              NibblesUsed++;
              }
            }
          PackedData[2+(i>>1)]|=(HalfBitEntry<<((i & 1)*4));
          }
        return(((NibblesUsed+1)>>1)+2+TotalBitEntries);
        }

DWORD   PackPatternDPCM20(ChannelPattern *NT2_Pattern, BYTE *PackedData)
        {
        int     Note, Oct, Inst, Eff, EffParm, Tone, SmpNum;
        int     i, j;
        DWORD   PackedSize=0;
        DWORD   PackedSnote;
        if(PatternIsEmpty(NT2_Pattern)) return(0);


        for(i=0; i<64; i++) {
          Note=NT2_Pattern->Row[i].note;
          Oct=NT2_Pattern->Row[i].octave;
          Inst=NT2_Pattern->Row[i].inst_num2;
          //Eff=eph2nedeph[NT2_Pattern->Row[i].ephphect];
          //EffParm=(NT2_Pattern->Row[i].e_parm1<<4)+NT2_Pattern->Row[i].e_parm2;
          Eff=0;
          EffParm=0;

          //if(!Note) Tone=0;
          //else Tone=Note+3;
          if((!Note) || (!Inst)) {
                Tone=0;
                SmpNum=0;
                Inst=0;
                }
          else {
                Tone=(NoteTable[Inst-1][Oct][Note-1] & 0xF);
                SmpNum=(NoteTable[Inst-1][Oct][Note-1]>>4);
                }

          PackedSnote=Tone+(SmpNum<<4)+(Inst<<8)+(Eff<<12)+(EffParm<<16);

          *PackedData=(BYTE)(PackedSnote & 0xFF);
          PackedData++;
          *PackedData=(BYTE)((PackedSnote>>8) & 0xFF);
          PackedData++;
          *PackedData=(BYTE)((PackedSnote>>16) & 0xFF);
          PackedData++;
          PackedSize+=3;
          }
        return(PackedSize);
        }


DWORD   PackInstrumentDPCM20(InstDPCM *TheInst, BYTE *Dest)
        {
        int     i, j;
        //Dest[0]=TheInst->Length;
        //Dest[1]=TheInst->LoopStart;
        //Dest[2]=TheInst->LoopLength;
        //Dest[3]=TheInst->Volume;
        Dest[0]=TheInst->Sample[0].Volume;
        //TheInst->Volume;
        for(i=0; i<MAX_DPCM_SAMPLES; i++) {
          Dest[1+2*i]=TheInst->Sample[i].Length;
          Dest[1+2*i+1]=TheInst->Sample[i].LoopStart+(TheInst->Sample[i].LoopFlag<<7);
          }
        return(17);
        }

DWORD   UnPackPattern20(BYTE *PackedData, ChannelPattern *NT2_Pattern, int Chn)
        {
        int     Note, Oct, Inst, Eff, EffParm, Tone, SmpNum;
        int     i, j;
        DWORD   PackedSize=0;
        DWORD   PackedSnote;
        int     NibPos=0;
        int     StartRow=PackedData[0],
                EndRow=PackedData[1];
        BYTE    *BitEntry,
                *PatData;

        ClearPattern(NT2_Pattern);

        BitEntry=PackedData+2;
        PatData=PackedData+2+((((EndRow+1)-StartRow)+1)>>1);

        for(i=StartRow; i<=EndRow; i++) {
          int   EntryPos=i-StartRow;
          int   ByteOffs=(EntryPos>>1),
                OddNib=(EntryPos & 1);

          Note=0;
          Oct=0;
          Tone=0;
          Inst=0;
          Eff=0;
          EffParm=0;

          if((BitEntry[ByteOffs]>>(4*OddNib)) & 1) {
            if(Chn==NOSWAV_CHN) {
              Tone|=(15-(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF))+1;
              NibPos++;
              }
            else {
              if((NibPos & 1)==0) {
                Tone|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF);
                //Tone|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF)<<4;
                NibPos++;
                Tone|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF)<<4;
                //Tone|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
                NibPos++;
                }
              else {
                Tone|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF)<<4;
                //Tone|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF)<<4;
                NibPos++;
                Tone|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
                //Tone|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
                NibPos++;
                }
                
              }
            }
          if((BitEntry[ByteOffs]>>(4*OddNib)) & 2) {
            Inst=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF)+1;
            NibPos++;
            }
          if((BitEntry[ByteOffs]>>(4*OddNib)) & 4) {
            Eff=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
            NibPos++;
            }
          if((BitEntry[ByteOffs]>>(4*OddNib)) & 8) {
            if((NibPos & 1)==0) {
              EffParm|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF);
              //EffParm|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF)<<4;
              NibPos++;
              EffParm|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF)<<4;
              //EffParm|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
              NibPos++;
              }
            else {
              EffParm|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF)<<4;
              //EffParm|=(PatData[(NibPos>>1)]>>(4*(NibPos & 1)) & 0xF)<<4;
              NibPos++;
              EffParm|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
              //EffParm|=((PatData[(NibPos>>1)]>>(4*(NibPos & 1))) & 0xF);
              NibPos++;
              }
            }

          if(Tone) {
            if(Tone==97) {
              Oct=0;
              Note=NOTE_OPHPH;
              }
            else {
              Oct=(int)((Tone-1)/12);
              Note=(int)((Tone-1)-Oct*12)+1;
              }
            }
          else
            {
            Note=0;
            Oct=0;
            }

          NT2_Pattern->Row[i].note=Note;
          NT2_Pattern->Row[i].octave=Oct;
          NT2_Pattern->Row[i].inst_num1=(Inst & 0x10)>>4;
          NT2_Pattern->Row[i].inst_num2=(Inst & 0x0F);
          NT2_Pattern->Row[i].ephphect=Eff;
          NT2_Pattern->Row[i].e_parm1=(EffParm & 0xF0)>>4;
          NT2_Pattern->Row[i].e_parm2=(EffParm & 0x0F);
          }
        return(0);
        }


DWORD   UnPackPatternDPCM20(BYTE *PackedData, ChannelPattern *NT2_Pattern)
        {
        int     Note=0, Oct=0, Inst, Eff, EffParm, Tone, SmpNum;
        int     i, j, k;
        DWORD   PackedSize=0;
        DWORD   PackedSnote;



       for(i=0; i<64; i++) {
          PackedSnote=PackedData[0]+(PackedData[1]<<8)+(PackedData[2]<<16);

          Tone=(PackedSnote & 0xF);
          SmpNum=((PackedSnote>>4) & 0x7);
          Inst=((PackedSnote>>8) & 0x0F);
          Eff=((PackedSnote>>12) & 0x07);
          EffParm=((PackedSnote>>16) & 0xFF);

          
          

          if(Inst) {
              Oct=0;
              Note=1;

            for(j=0; j<MAX_DPCM_OCTAVES; j++)
              for(k=0; k<12; k++) {
                if(NoteTable[Inst-1][j][k]==(Tone+(SmpNum<<4))) {
                  Oct=j;
                  Note=k+1;}
                }
            }
          else
            {
            Note=0;
            Oct=0;
            }

          NT2_Pattern->Row[i].note=Note;
          NT2_Pattern->Row[i].octave=Oct;
          NT2_Pattern->Row[i].inst_num1=0;
          NT2_Pattern->Row[i].inst_num2=(Inst & 0x0F);
          NT2_Pattern->Row[i].ephphect=Eff;
          NT2_Pattern->Row[i].e_parm1=(EffParm & 0xF0)>>4;
          NT2_Pattern->Row[i].e_parm2=(EffParm & 0x0F);

          PackedData+=3;
          PackedSize+=3;
          }
        return(PackedSize);
        }


DWORD   UnPackInstrument20(NedInstStrucNew *Src, struct inst_struc *Dest)
        {

        (*Dest).ArpX=(*Src).ArpXY & 0xF;
        (*Dest).ArpY=((*Src).ArpXY>>4) & 0xF;
        (*Dest).ArpZ=(*Src).ArpZAndMisc & 0xF;
        (*Dest).duty_cycle=(*Src).CR0>>6;
        (*Dest).hold_note=((*Src).CR0>>5) & 1;
        (*Dest).envelope_phixed=((*Src).CR0>>4) & 1;
        (*Dest).VibratoDepth=(*Src).AutoVibrato & 0xF;
        (*Dest).VibratoSpeed=((*Src).AutoVibrato>>4) & 0xF;
        (*Dest).TremoloDepth=(*Src).AutoTremolo & 0xF;
        (*Dest).TremoloSpeed=((*Src).AutoTremolo>>4) & 0xF;
        (*Dest).volume=((*Src).VolumeAndVolSlide>>4) & 0xF;
        (*Dest).VolumeSlide=(*Src).VolumeAndVolSlide & 0xF;
        (*Dest).VolumeSlideDir=((*Src).TimeLengthAndVSlideDir>>7) & 1;
        (*Dest).active_tlength=(*Src).TimeLengthAndVSlideDir & 0x7F;
        (*Dest).LoopedNoise=((*Src).ArpZAndMisc>>6) & 1;
        (*Dest).ReversedArpeggio=((*Src).ArpZAndMisc>>7) & 1;
        (*Dest).NonLoopedArpeggio=((*Src).ArpZAndMisc>>5) & 1;

        (*Dest).PortaDir=((*Src).Porta>>1) & 1;
        if((*Dest).PortaDir)
          (*Dest).Porta=-((*Src).Porta & 0x7F);
        else
          (*Dest).Porta=(*Src).Porta & 0x7F;

        return(8);
        }

DWORD   UnPackInstrument10(struct ned_inst_struc *Src, struct inst_struc *Dest)
        {
        // Control reg #1
        Dest->duty_cycle=(Src->creg1>>6);
        Dest->hold_note=((Src->creg1 & 0x20)>>5);
        Dest->envelope_phixed=((Src->creg1 & 0x10)>>4);
        Dest->volume=(Src->creg1 & 0x0f);
        // Control reg #2
        Dest->variable_phreq=(Src->creg2>>7);
        Dest->phq_changespd=((Src->creg2 & 0x70)>>4);
        Dest->ishi2lo=((Src->creg2 & 0x08)>>3);
        Dest->phq_range=(Src->creg2 & 0x07);
        // Control reg #4
        Dest->active_tlength=LCounterTable[((Src->creg4 & 0xf8)>>3)];

        (*Dest).ArpX=0;
        (*Dest).ArpY=0;
        (*Dest).ArpZ=0;
        (*Dest).VibratoDepth=0;
        (*Dest).VibratoSpeed=0;
        (*Dest).TremoloDepth=0;
        (*Dest).TremoloSpeed=0;
        (*Dest).VolumeSlide=0;
        (*Dest).VolumeSlideDir=0;
        return(3);
        }

DWORD   UnPackInstrumentDPCM20(BYTE *Src, InstDPCM *TheInst)
        {
        int     i, j;
        //TheInst->Length=Src[0];
        //TheInst->LoopStart=Src[1];
        //TheInst->LoopLength=Src[2];
        //TheInst->Volume=Src[3];

        //TheInst->Volume=Src[0];
        for(i=0; i<MAX_DPCM_SAMPLES; i++) {
          TheInst->Sample[i].Volume=Src[0];
          TheInst->Sample[i].Length=Src[1+i*2];
          TheInst->Sample[i].LoopStart=(Src[1+i*2+1] & 0x3F);
          TheInst->Sample[i].LoopFlag=Src[1+i*2+1]>>7;
          }
        return(17);
        }

DWORD   UnPackOrderEntry20(BYTE *Src, BYTE *Dest)
        {
        int     i, j;
        DWORD   PackedOrderEntry=Src[0]+(Src[1]<<8)+(Src[2]<<16)+(Src[3]<<24);
        Dest[SQRWAV1_CHN]=(PackedOrderEntry & 0x1F);
        Dest[SQRWAV2_CHN]=((PackedOrderEntry>>5) & 0x1F);
        Dest[TRIWAV_CHN]=((PackedOrderEntry>>10) & 0x1F);
        Dest[NOSWAV_CHN]=((PackedOrderEntry>>15) & 0x1F);
        Dest[DPCM_CHN]=((PackedOrderEntry>>20) & 0x0F);
        return(3);
        }

void    nt_make_nes(char *ned_name, char *nes_name)
        {
        FILE    *ned_phile, *nes_phile, *prg_phile;
        unsigned char   *ned_buphpher, *prg_buphpher, *null_data;
        int     ned_phile_size, nes_phile_size, prg_phile_size;
        unsigned short  nmi_vector=0x8013, reset_vector=0x8000, brk_vector=0x8000; 

        ned_buphpher=(unsigned char *)malloc(32768);
        prg_buphpher=(unsigned char *)malloc(16384);
        null_data=(unsigned char *)malloc(32768);

        memset(null_data, 0, 32768);


        if(!(ned_phile=fopen(ned_name, "rb"))) exit(1);
        if(!(nes_phile=fopen(nes_name, "wb"))) exit(1);

        if(!(prg_phile=fopen("replay.bin", "rb")))
          {
          printf("apsvett!!!");
          exit(1);
          }

        fseek(ned_phile, 0, SEEK_END);
        ned_phile_size=ftell(ned_phile);
        fseek(prg_phile, 0, SEEK_END);
        prg_phile_size=ftell(prg_phile);
        
        printf("\n%d\n", ned_phile_size);


        fseek(ned_phile, 0, SEEK_SET);
        fseek(prg_phile, 0, SEEK_SET);
        

        fread(ned_buphpher, 1, ned_phile_size, ned_phile);
        fread(prg_buphpher, 1, prg_phile_size, prg_phile);

        fwrite(&nes_header, 16, 1, nes_phile);
        fwrite(prg_buphpher, 1, prg_phile_size, nes_phile);

        
        fwrite(ned_buphpher, 1, ned_phile_size, nes_phile);
        nes_phile_size=ftell(nes_phile);
        

        fwrite(null_data, 1, (32762+16)-(nes_phile_size), nes_phile);

        fwrite(&nmi_vector, 2, 1, nes_phile);
        fwrite(&reset_vector, 2, 1, nes_phile);
        fwrite(&brk_vector, 2, 1, nes_phile);
        fwrite(null_data, 1, 8192, nes_phile);

        fclose(prg_phile);
        fclose(nes_phile);
        fclose(ned_phile);

        free(null_data);
        free(prg_buphpher);
        free(ned_buphpher);
        }


void    CopyOrderEntry(int Src, int Dest)
        {
        int     i;
        for(i=0; i<MAX_CHANNELS; i++) OrderList[Dest][i]=OrderList[Src][i];
        }

void    ClearOrderEntry(int Entry)
        {
        memset(OrderList[Entry], 0, 5);
        }

void    CopyPattern(ChannelPattern *Src, ChannelPattern *Dest)
        {
        memcpy(Dest, Src, sizeof(ChannelPattern));
        }

void    ClearPattern(ChannelPattern *Pat)
        {
        memset(Pat, 0, sizeof(ChannelPattern));
        }

int     PatternIsEmpty(ChannelPattern *Pat)
        {
        int     i, j;
        struct  snote_struc *S;
        for(i=0; i<64; i++) {
          S=&Pat->Row[i];
          if(S->note || S->inst_num1 || S->inst_num2 || S->ephphect)
                return(0);
          }
        return(1);
        }


void    ClearOrder2Max(int StartEntry)
        {
        int     i;
        if(i>=NT_MAX_ORDER) return;
        for(i=StartEntry; i<NT_MAX_ORDER; i++)
                ClearOrderEntry(i);
        }


void    DrawBgColLine_H(int StartX, int EndX, int Y, BYTE Color, BYTE *Buffer)
        {
        int     i, j;
        int     Offset=Y*160;
        for(i=StartX; i<=EndX; i++)
                Buffer[Offset+i*2+1]=((Buffer[Offset+i*2+1] & 0x0F) | (Color<<4));
        }


void    DrawBgColLine_V(int X, int StartY, int EndY, BYTE Color, BYTE *Buffer)
        {
        int     i, j;
        int     Offset=StartY*160+X*2+1;
        for(i=StartY; i<=EndY; i++)
                Buffer[Offset+i*160]=((Buffer[Offset+i*160] & 0x0F) | (Color<<4));
        }

void    DrawFgColLine_H(int StartX, int EndX, int Y, BYTE Color, BYTE *Buffer)
        {
        int     i, j;
        int     Offset=Y*160;
        for(i=StartX; i<=EndX; i++)
                Buffer[Offset+i*2+1]=((Buffer[Offset+i*2+1] & 0xF0) | Color);
        }

void    DrawBothColLine_H(int StartX, int EndX, int Y, BYTE Color, BYTE *Buffer)
        {
        int     i, j;
        int     Offset=Y*160;
        for(i=StartX; i<=EndX; i++)
                Buffer[Offset+i*2+1]=Color;
        }

void    DrawFgColLine_V(int X, int StartY, int EndY, BYTE Color, BYTE *Buffer)
        {
        int     i, j;
        int     Offset=StartY*160+X*2+1;
        for(i=StartY; i<=EndY; i++)
                Buffer[Offset+i*160]=((Buffer[Offset+i*160] & 0xF0) | Color);
        }


void    Blit(int x, int y, int Width, int Height, BYTE *Src, BYTE *Dest)
        {
        int     i, j;
        WORD    *W_Src=(WORD *)Src;
        WORD    *W_Dest=(WORD *)Dest;
        for(i=0; i<Height; i++)
          for(j=0; j<Width; j++)
            W_Dest[(y+i)*scrtxt_ScreenWidth+(x+j)]=W_Src[i*Width+j];
        }

void    PrintStringBoth(int x, int y, char *Str, BYTE Color, void *Dest)
        {
        BYTE    *B_Dest=((BYTE *)Dest)+y*scrtxt_ScreenWidth*2+x*2;
        int     i=0;
        while(Str[i]) {
          B_Dest[i*2]=Str[i];
          B_Dest[i*2+1]=Color;
          i++;
          }
        }
        
void    ResetEditors(void)
        {
        int     i, j;

        // Reset patterns
        for(i=0; i<MAX_A_CHANNELS; i++)
                for(j=0; j<MAX_PATTERNS; j++)
                        ClearPattern(&ChnPattern[i][j]);
                for(j=0; j<MAX_PATTERNS_DPCM; j++)
                        ClearPattern(&ChnPattern[DPCM_CHN][j]);

        // Reset Order List
        for(i=0; i<NT_MAX_ORDER; i++)
                ClearOrderEntry(i);

        // Reset Synth Instruments
        for(i=0; i<MAX_INSTRUMENTS; i++)
                memset(inst, 0, MAX_INSTRUMENTS*sizeof(struct inst_struc));
       
        // Reset DPCM Instruments
        for(i=0; i<MAX_INSTRUMENTS_DPCM; i++)
          for(j=0; j<MAX_DPCM_SAMPLES; j++)
                if(SampleInst[i].Sample[j].SamplePtr) free(SampleInst[i].Sample[j].SamplePtr);
        memset(SampleInst, 0, MAX_INSTRUMENTS_DPCM*sizeof(InstDPCM));

        memset(InstName, 0, MAX_INSTRUMENTS_DPCM*DPCM_INAME_LENGTH);
        memset(SampleName, 0, MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES*DPCM_INAME_LENGTH);
        // Reset DPCM Note Tables
        memset(NoteTable, 0, MAX_INSTRUMENTS_DPCM*MAX_DPCM_OCTAVES*12);
        }
