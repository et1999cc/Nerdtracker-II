#include "typedefs.h"

#define DMA_MASKREG             0x0A
#define DMA_MODEREG             0x0B
#define DMA_CLEARPTRSREG        0x0C

#define DMA_PAGEREG_DMAC0       0x87
#define DMA_PAGEREG_DMAC1       0x83
#define DMA_PAGEREG_DMAC2       0x81
#define DMA_PAGEREG_DMAC3       0x82

#define DMA_MODE_DEMAND         0x00
#define DMA_MODE_SIGNAL         0x40
#define DMA_MODE_BLOCK          0x80
#define DMA_MODE_CASCADE        0xC0

#define DMA_AUTOINIT            0x10

#define DMA_OPERATION_VERIFY    0x00
#define DMA_OPERATION_WRITE     0x04
#define DMA_OPERATION_READ      0x08


