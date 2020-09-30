#include <stdio.h>

#include "dephs.h"
#include "scrtxt.h"


unsigned char *scrtxt_vidbuphph=NULL;
unsigned char *scrtxt_vidmem=(unsigned char *)((0xb8000));
int           scrtxt_ScreenWidth=80;
int           scrtxt_ScreenHeight=50;


char    cursor_blink=1;

        void    scrtxt_copy2vidmem_asm(void);
        void    scrtxt_set_hires_asm(void);
        void    scrtxt_set_lores_asm(void);
        void    scrtxt_write_char_asm(int x, int y, char the_char, char color, char bcolor, char blink);
        void    scrtxt_write_col_asm(int x, int y, char color, char blink);
        void    scrtxt_put_pic_asm(void *pic);
        void    scrtxt_put_pic_lines_asm(void *pic, int lines);
        void    scrtxt_put_pic_lines_clear_asm(void *pic, int lines);
        void    scrall_wait_vr_asm(void);
        void    scrtxt_set_cursor_pos_asm(int x, int y);
        void    scrtxt_set_blink_asm(int Mode);

// Sets high-resolution textmode
        void    scrtxt_set_hires(void)
                {
                scrtxt_set_hires_asm();
                }
                  
                

// Sets low-resolution textmode
        void    scrtxt_set_lores(void)
                {
                scrtxt_set_lores_asm();
                }


        void    scrtxt_set_blink(int Mode)
                {
                scrtxt_set_blink_asm(Mode);
                //int     OldSelect, Value;
                //inp(0x3DA);
                //outp(0x3C0, 0x10);
                //outp(0x3C0, 0x0C);
                //OldSelect=((inp(0x3C0) & 0xF0) | 0x10);
                //outp(0x3C0, OldSelect);
                //Value=((inp(0x3C0) & 0xF7) | (Mode<<3));
                //outp(0x3C0, OldSelect);
                //outp(0x3C0, Value);
                }

// Writes a character to the speciphied position with speciphied attributes
        void    scrtxt_write_char(int x, int y, char the_char, char color, char bcolor, char blink)
                {
scrtxt_write_char_asm(x, y, the_char, color, bcolor, blink);
                }
        
// Writes a character to the speciphied position with speciphied color
        void    scrtxt_write_col(int x, int y, char color, char blink)
                {
                scrtxt_write_col_asm(x, y, color, blink);
                }


// Copies *pic to the screen
        void    scrtxt_put_pic(void *pic)
                {
                scrtxt_put_pic_asm(pic);
                }

// Copies *pic to the screen with the lines
        void    scrtxt_put_pic_lines(void *pic, int lines)
                {
                scrtxt_put_pic_lines_asm(pic, lines);
                }

// Copies *pic to the screen with the lines
        void    scrtxt_put_pic_lines_clear(void *pic, int lines)
                {
                scrtxt_put_pic_lines_clear_asm(pic, lines);
                }

// Waits for a vertical retrace
        void    scrall_wait_vr(void)
                {
                scrall_wait_vr_asm();
                }

// Sets the position oph the cursor
        void    scrtxt_set_cursor_pos(int x, int y)
                {
                scrtxt_set_cursor_pos_asm(x, y);
                }






        #pragma aux scrtxt_set_hires_asm =  \
        "mov    eax, 1112h"             \
        "mov    ebx, 0800h"             \
        "int    10h"                    \
        modify  [eax ebx ecx edx esi edi];


        #pragma aux scrtxt_set_lores_asm =  \
        "mov    eax, 0003h"             \
        "int    10h"                    \
        modify  [eax ebx ecx edx esi edi];




        #pragma aux scrtxt_write_char_asm = \
        "mov    edi, ebx"               \
        "shl    ebx, 7"                 \
        "shl    edi, 5"                 \
        "shl    eax, 1"                 \
        "add    edi, ebx"               \
        "add    edi, eax"               \
        "add    edi, scrtxt_vidmem"     \
        "shl    dl, 4"                  \
        "shl    dh, 7"                  \
        "or     ch, dl"                 \
        "or     ch, dh"                 \
        "mov    [edi], cx"              \
        modify  [edi]                   \
        parm    [eax] [ebx] [cl] [ch] [dl] [dh];
        

        #pragma aux scrtxt_write_col_asm =  \
        "mov    edi, ebx"               \
        "shl    ebx, 7"                 \
        "shl    edi, 5"                 \
        "shl    eax, 1"                 \
        "add    edi, ebx"               \
        "add    edi, eax"               \
        "add    edi, scrtxt_vidmem"     \
        "shl    ch, 7"                  \
        "mov    al, [edi+1]"            \
        "and    al, 070h"               \
        "or     al, cl"                 \
        "or     al, ch"                 \
        "mov    [edi+1], al"            \
        modify  [edi]                   \
        parm    [eax] [ebx] [cl] [ch];



                    
        #pragma aux scrtxt_put_pic_asm =    \
        "mov    edi, scrtxt_vidmem"     \
        "mov    ecx, 1992"              \
        "rep    movsd"                  \
        "mov    ecx, 8"                 \
        "xor    eax, eax"               \
        "rep    stosd"                  \
        modify  [eax ecx esi edi]       \
        parm    [esi];

        #pragma aux scrtxt_put_pic_lines_asm =    \
        "mov    edi, scrtxt_vidmem"     \
        "mov    ecx, eax"               \
        "shl    eax, 5"                 \
        "shl    ecx, 3"                 \
        "add    ecx, eax"               \
        "rep    movsd"                  \
        modify  [eax ecx esi edi]       \
        parm    [esi] [eax];

        #pragma aux scrtxt_put_pic_lines_clear_asm =    \
        "mov    edi, scrtxt_vidmem"     \
        "mov    ecx, eax"               \
        "shl    eax, 5"                 \
        "shl    ecx, 3"                 \
        "mov    edx, 2000"              \
        "sub    edx, ecx"               \
        "add    ecx, eax"               \
        "rep    movsd"                  \
        "cmp    edx, 0"                 \
        "je     no_more"                \
        "mov    ecx, edx"               \
        "xor    eax, eax"               \
        "rep    stosd"                  \
        "no_more:"                      \
        modify  [eax ecx edx esi edi]   \
        parm    [esi] [eax];



        #pragma aux scrall_wait_vr_asm =   \
        "mov    dx, 3dah"       \
        "loop1:"                \
        "in     al, dx"         \
        "test   al, 8"          \
        "jz     loop1"          \
        "loop2:"                \
        "in     al, dx"         \
        "test   al, 8"          \
        "jnz    loop2"          \
        modify  [eax edx];


        #pragma aux scrtxt_set_cursor_pos_asm = \
        "pushad"                \
        "push   eax"            \
        "push   ebx"            \
        "mov    ah, 01h"        \
        "mov    ch, cursor_blink"          \
        "shl    ch, 5"          \
        "mov    cl, 7"          \
        "int    10h"            \
        "pop    ebx"            \
        "pop    eax"            \
        "xor    edx, edx"       \
        "mov    dl, al"         \
        "mov    dh, bl"         \
        "mov    eax, 0200h"     \
        "xor    ebx, ebx"       \
        "int    10h"            \
        "popad"                 \
        modify  [eax ebx ecx edx esi edi] \
        parm    [eax] [ebx];


        void    scrtxt_copy2vidmem(void)
                {
                scrtxt_copy2vidmem_asm();
                }

        #pragma aux scrtxt_copy2vidmem_asm = \
        "pushad"                        \
        "mov    esi, scrtxt_vidmem"     \
        "mov    edi, 0b8000h"           \
        "mov    ecx, 2000"              \
        "rep    movsd"                  \
        "xor    eax, eax"               \
        "stosd"                         \
        "stosd"                         \
        "stosd"                         \
        "stosd"                         \
        "stosd"                         \
        "popad";                        \
        
        #pragma aux scrtxt_set_blink_asm = \
        "pushad"                \
        "mov    eax, 1003h"     \
        "mov    ebx, 01h"       \
        "int    10h"            \
        "popad"                 \
        parm    [ebx];



