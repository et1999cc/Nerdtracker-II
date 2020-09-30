#include <io.h>

#include "dma.h"


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
        _disable();

        // Set mask of DMA channel #DMAC
        outp(DMAC_IOBase+(DMA_MASKREG<<ChnHi), (0x04 | ChnBits));
        // Clear internal pointers
        outp(DMAC_IOBase+(DMA_CLEARPTRSREG<<ChnHi), 0x00);
        // Set transfer mode
        outp(DMAC_IOBase+(DMA_MODEREG<<ChnHi), (Mode | Operation | ChnBits));
        

        // Set page
        outp(PageReg[ChnBits]+(ChnHi<<3), Page);

        // Set offset
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi)), (Offset & 0xFF));
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi)), (Offset>>8));

        // Set length
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi))+(1<<ChnHi), ((Length-1) & 0xFF));
        outp(DMAC_IOBase+(ChnBits<<(1+ChnHi))+(1<<ChnHi), ((Length-1)>>8));

        // Clear mask of DMA channel #DMAC
        outp(DMAC_IOBase+(DMA_MASKREG<<ChnHi), ChnBits);

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
