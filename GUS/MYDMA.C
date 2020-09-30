#include <io.h>
#include <conio.h>
#include <i86.h>

/* #include "osproto.h"
#include "forte.h"
#include "dma.h"
#include "gf1proto.h"
#include "extern.h"
#include "ultraerr.h"*/

#include "mydma.h"

// extern DMA_ENTRY _gf1_dma[];

typedef struct
{
    char page;
    unsigned int offset;
    unsigned int length;
} DMA_block;

/* Just helps in making things look cleaner.  :) */
typedef unsigned char   uchar;
typedef unsigned int    uint;

/* Defines for accessing the upper and lower byte of an integer. */
#define LOW_BYTE(x)         (x & 0x00FF)
#define HI_BYTE(x)          ((x & 0xFF00) >> 8)

/* Quick-access registers and ports for each DMA channel. */
uchar MaskReg[8]   = { 0x0A, 0x0A, 0x0A, 0x0A, 0xD4, 0xD4, 0xD4, 0xD4 };
uchar ModeReg[8]   = { 0x0B, 0x0B, 0x0B, 0x0B, 0xD6, 0xD6, 0xD6, 0xD6 };
uchar ClearReg[8]  = { 0x0C, 0x0C, 0x0C, 0x0C, 0xD8, 0xD8, 0xD8, 0xD8 };

uchar PagePort[8]  = { 0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A };
uchar AddrPort[8]  = { 0x00, 0x02, 0x04, 0x06, 0xC0, 0xC4, 0xC8, 0xCC };
uchar CountPort[8] = { 0x01, 0x03, 0x05, 0x07, 0xC2, 0xC6, 0xCA, 0xCE };

void LoadPageAndOffset(DMA_block *blk, char *data)
{
    unsigned int temp, segment, offset;
    unsigned long foo;

    segment = FP_SEG(data);
    offset  = FP_OFF(data);

//    blk->page = (segment & 0xF000) >> 12;
    blk->page = ((long)data)>>16;
//    temp = (segment & 0x0FFF) << 4;
//    foo = offset + temp;
//    if (foo > 0xFFFF)
//        blk->page++;
//    blk->offset = (unsigned int)foo;
    blk->offset = (((long)data) & 0xFFFF);
}


void StartDMA(uchar DMA_channel, DMA_block *blk, uchar mode)
{
    /* First, make sure our 'mode' is using the DMA channel specified. */
    mode |= DMA_channel;

    /* Don't let anyone else mess up what we're doing. */
    _disable();

    /* Set up the DMA channel so we can use it.  This tells the DMA */
    /* that we're going to be using this channel.  (It's masked) */
    outp(MaskReg[DMA_channel], 0x04 | DMA_channel);

    /* Clear any data transfers that are currently executing. */
    outp(ClearReg[DMA_channel], 0x00);

    /* Send the specified mode to the DMA. */
    outp(ModeReg[DMA_channel], mode);

    /* Send the physical page that the data lies on. */
    outp(PagePort[DMA_channel], blk->page);


    /* Send the offset address.  The first byte is the low base offset, the */
    /* second byte is the high offset. */
    outp(AddrPort[DMA_channel], LOW_BYTE(blk->offset));
    outp(AddrPort[DMA_channel], HI_BYTE(blk->offset));


    /* Send the length of the data.  Again, low byte first. */
    outp(CountPort[DMA_channel], LOW_BYTE(blk->length));
    outp(CountPort[DMA_channel], HI_BYTE(blk->length));

    /* Ok, we're done.  Enable the DMA channel (clear the mask). */
    outp(MaskReg[DMA_channel], DMA_channel);

    /* Re-enable interrupts before we leave. */
    _enable();
}



BYTE    PageReg[4]={DMA_PAGEREG_DMAC0,
                    DMA_PAGEREG_DMAC1,
                    DMA_PAGEREG_DMAC2,
                    DMA_PAGEREG_DMAC3};

void    DMA_SetDMAC(int DMAC, long Addr, long Length, int Mode, int Operation)
        {
        long    Page=((Addr & 0xF0000)>>16), Offset=(Addr & 0xFFFF);
        int     ChnBits=(DMAC & 0x03),
                ChnHi=(DMAC>>2),
                DMAC_IOBase=(0xC0*ChnHi);
// DMA_ENTRY *tdma;


        _disable();

/* tdma  = &_gf1_dma[DMAC-1];

tdma->flags&=(~DMA_PENDING);
PrimeDma((void far *)Addr,4,Length,DMAC);*/


// VERIFY
        // Set mask of DMA channel #DMAC
        outp(DMAC_IOBase+(DMA_MASKREG<<ChnHi), (0x04 | ChnBits));
        // Clear internal pointers
        outp(DMAC_IOBase+(DMA_CLEARPTRSREG<<ChnHi), 0x00);
        // Set transfer mode
        outp(DMAC_IOBase+(DMA_MODEREG<<ChnHi), (Mode | Operation | ChnBits));
// VERIFY        

        // Set offset
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi)), ((Offset>>ChnHi) & 0xFF));
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi)), ((Offset>>ChnHi)>>8));

        // Set page
        outp(PageReg[ChnBits]+(ChnHi<<3), Page);
        // Double-write perhaps? (IMS does this...)
        outp(PageReg[ChnBits]+(ChnHi<<3), Page);


        // Set length
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi))+(1<<ChnHi), (((Length>>ChnHi)-1) & 0xFF));
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi))+(1<<ChnHi), (((Length>>ChnHi)-1)>>8));

        // Clear mask of DMA channel #DMAC
// VERIFY
        outp(DMAC_IOBase+(DMA_MASKREG<<ChnHi), ChnBits);
// VERIFY
        _enable();
        }




/*
uint DMAComplete(uchar DMA_channel)
{

    register int z;

    z = CountPort[DMA_channel];
    outportb(0x0C, 0xFF);

    do {
    int x=0, y=0, z;
    x=inp(0x03);
    x|=(inp(0x03)<<8);

    y=inp(0x03);
    y|=(inp(0x03)<<8);

    z=
    

redo:
    asm {
        mov  dx,z
        in   al,dx
		mov	 bl,al
		in	 al,dx
		mov  bh,al

		in	 al,dx
		mov	 ah,al
		in	 al,dx
		xchg ah,al

        sub  bx,ax
        cmp  bx,40h
		jg	 redo
		cmp	 bx,0FFC0h
		jl	 redo
	}
    return _AX;
}

*/
