#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <mem.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>

#include "typedefs.h"
#include "dephs.h"
#include "file.h"
#include "scrtxt.h"
#include "nt2.h"
#include "system.h"
#include "keyb.h"

void    main(int argc, char *argv[])
        {
        int             i, j, k;
        BYTE            *BytePtr;
        //WORD            *WordPtr;
        int             NumSampleOffsets=0, NumSampleNames=0;
        DWORD           SampleOffset[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES+1];
        DWORD           SampleSize[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES];
        BYTE            NesOffset[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES];
        BYTE            DPCM_ToneTable[MAX_INSTRUMENTS_DPCM*256];
        DWORD           TotalSampleSize=0;
        DWORD           ChunkSize=0;
        DWORD           FileSize;
        InfoHeader      Info;
        BYTE            *DestData;
        DWORD           DestOffset=0;
        void            *SampleChunkData;
        DWORD           SampleChunkSize;
        SampInstInfo    *InstP=NULL;
        int             NumInstruments;
        BYTE            Zero=0;
        WORD            BaseOffset=0xC000;

        if((argc<4) || (argc>5)) {
          printf("Usage: SORTSMPS <NED header file> <sample offsets file> <sample data file> <offset in hex ( >= $C000)>\n");
          exit(1);}

        if(argc==5)
          sscanf(argv[4], "%X", &BaseOffset);

        FileSize=get_filesize(argv[1]);

        readfile(argv[1], 0, sizeof(InfoHeader), &Info);
        SampleChunkSize=Info.NumSampleInstrumentsNTSC*sizeof(SampInstInfo)+Info.SampleDataSizeNTSC;
        SampleChunkData=(void *)malloc(SampleChunkSize);
        readfile(argv[1], sizeof(InfoHeader),SampleChunkSize,SampleChunkData);

        BytePtr=(BYTE *)SampleChunkData;
        BytePtr+=Info.NumSampleInstrumentsNTSC*sizeof(SampInstInfo);
        InstP=(SampInstInfo *)SampleChunkData;

        //WordPtr=(WORD *)SampleChunkData;
        
        TotalSampleSize=Info.SampleDataSizeNTSC;
        NumInstruments=Info.NumSampleInstrumentsNTSC;
        

        //printf("Total Size: %d\n", (int)TotalSampleSize);


        for(i=0; i<NumInstruments; i++)
          for(j=0; j<InstP[i].NumSamples; j++)
            SampleOffset[i*MAX_DPCM_SAMPLES+j]=InstP[i].Sample[j].Offset;

        SampleOffset[NumInstruments*MAX_DPCM_SAMPLES]=TotalSampleSize;

        for(i=0; i<(NumInstruments*MAX_DPCM_SAMPLES); i++) {
          if(SampleOffset[i]==0xFFFFFFFF)
            SampleSize[i]=0;
          else {
            j=1;
            while(SampleOffset[i+j]==0xFFFFFFFF) j++;
            SampleSize[i]=SampleOffset[i+j]-SampleOffset[i];
            }
          }


        NumSampleOffsets=NumInstruments*MAX_DPCM_SAMPLES;
        DestData=(BYTE *)malloc(TotalSampleSize+NumSampleOffsets*64);



        for(i=0; i<NumSampleOffsets; i++) {
          if(DestOffset & 63) {
                memset(&DestData[DestOffset], 0, 64-(DestOffset & 63));
                DestOffset+=(64-(DestOffset & 63));}
          if(SampleSize[i]) {
            NesOffset[i]=(DestOffset>>6);
            memcpy(&DestData[DestOffset], &BytePtr[SampleOffset[i]], SampleSize[i]);
            DestOffset+=SampleSize[i];}
          printf("Sample Size: %d\n", (int)SampleSize[i]);
          }


//        for(i=0; i<NumSampleOffsets; i++) {
      for(i=0; i<NumInstruments; i++) {
        //for(j=0; j<MAX_DPCM_SAMPLES; j++) {
        for(j=0; j<MAX_DPCM_OCTAVES; j++) {
        for(k=0; k<12; k++) {
          int   FreqVal=InstP[i].NoteTable[j][k] & 0xF;
          int   Smp=(InstP[i].NoteTable[j][k]>>4) & 0x7;
          int   Offset4012=NesOffset[i*MAX_DPCM_SAMPLES+Smp];
          int   Length4013=InstP[i].Sample[Smp].Length;

          if(InstP[i].Sample[Smp].LoopFlag)
            FreqVal|=0x40;

//          if(SampleSize[i*MAX_DPCM_SAMPLES+j]) {
          if(SampleSize[i*MAX_DPCM_SAMPLES+Smp]) {
            //NesOffset[i]=(DestOffset>>6);
            DPCM_ToneTable[i*256+(j*12+k)*4]=Length4013;
            DPCM_ToneTable[i*256+(j*12+k)*4+1]=Offset4012;
            //((BaseOffset-0xC000)+DestOffset)>>6;
            DPCM_ToneTable[i*256+(j*12+k)*4+2]=0x40;
            DPCM_ToneTable[i*256+(j*12+k)*4+3]=FreqVal;
            }
          }
          }
        }
        if(NumInstruments)
                writefile(argv[2], 0, NumInstruments*256, DPCM_ToneTable);
        else    writefile(argv[2], 0, 1, &Zero);

        if(DestOffset)
                writefile(argv[3], 0, DestOffset, DestData);
        else    writefile(argv[3], 0, 1, &Zero);

        //printf("Total size: %d\n", (int)DestOffset);
        }
