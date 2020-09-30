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
        int             i, j;
        BYTE            *BytePtr;
        WORD            *WordPtr;
        int             NumSampleOffsets=0, NumSampleNames=0;
        DWORD           SampleOffset[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES+1];
        DWORD           SampleSize[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES];
        BYTE            NesOffset[MAX_INSTRUMENTS_DPCM*MAX_DPCM_SAMPLES];
        DWORD           TotalSampleSize=0;
        DWORD           ChunkSize=0;
        DWORD           FileSize;
        InfoHeader      Info;
        BYTE            *DestData;
        DWORD           DestOffset=0;
        BYTE            *SampleChunkData;
        SampleInstInfo  *InstPtr=NULL;
        int             NumInstruments;
        BYTE            Zero=0;

        if(argc!=4) {
          printf("Usage: SORTSMPS <NED header file> <sample offsets file> <sample data file>\n");
          exit(1);}

        FileSize=get_filesize(argv[1]);

        readfile(argv[1], 0, sizeof(InfoHeader), &Info);
        SampleChunkData=(void *)malloc(Info.HeaderSize-sizeof(InfoHeader));
        readfile(argv[1], sizeof(InfoHeader),Info.HeaderSize-sizeof(InfoHeader),SampleChunkData);

        BytePtr=(BYTE *)SampleChunkData;
        WordPtr=(WORD *)SampleChunkData;
        
        TotalSampleSize=(int)BytePtr[0]+(((int)BytePtr[1])<<8)+(((int)BytePtr[2])<<16);
        NumInstruments=BytePtr[3];
        BytePtr+=4;
        InstPtr=(SampleInstInfo *)BytePtr;
        BytePtr+=NumInstruments*sizeof(SampleInstInfo);
        

        //printf("Total Size: %d\n", (int)TotalSampleSize);


        for(i=0; i<NumInstruments; i++)
          memcpy(&SampleOffset[i*MAX_DPCM_SAMPLES], InstPtr[i].SampleOffset, 4*MAX_DPCM_SAMPLES);
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
          if(DestOffset & 64) {
                memset(&DestData[DestOffset], 0, 64-(DestOffset & 64));
                DestOffset+=(64-(DestOffset & 64));}
          if(SampleSize[i]) {
            NesOffset[i]=(DestOffset>>6);
            memcpy(&DestData[DestOffset], &BytePtr[SampleOffset[i]], SampleSize[i]);
            DestOffset+=SampleSize[i];}
          //printf("Sample Size: %d\n", (int)SampleSize[i]);
          }
        if(NumInstruments)
                writefile(argv[2], 0, NumInstruments*MAX_DPCM_SAMPLES, NesOffset);
        else    writefile(argv[2], 0, 1, &Zero);
        if(DestOffset)
                writefile(argv[3], 0, DestOffset, DestData);
        else    writefile(argv[3], 0, 1, &Zero);
        //printf("Total size: %d\n", (int)DestOffset);
        }
