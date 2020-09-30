#include <conio.h>
#include <dos.h>

#include "dephs.h"
#include "system.h"

int     sys_timer_isr_installed=0;
long    sys_phrame_counter=0;
long    sys_timer_speed_add2=0;
long    sys_timer_speed;



int     sys_lepht_shipht_pressed;
int     sys_right_shipht_pressed;
int     sys_lepht_ctrl_pressed;
int     sys_right_ctrl_pressed;
int     sys_lepht_alt_pressed;
int     sys_right_alt_pressed;


void    (__interrupt __far *sys_timer_isr_bios)();


void    sys_do_misc_keys(void)
        {
        unsigned char *status1=(unsigned char *)0x417, *status2=(unsigned char *)0x418, *status3=(unsigned char *)0x496;

        if((*status1) & 0x2) sys_lepht_shipht_pressed=1;
          else sys_lepht_shipht_pressed=0;

        if((*status1) & 0x1) sys_right_shipht_pressed=1;
          else sys_right_shipht_pressed=0;

        if((*status2) & 0x2) sys_lepht_alt_pressed=1;
          else sys_lepht_alt_pressed=0;

        if((*status3) & 0x8) sys_right_alt_pressed=1;
          else sys_right_alt_pressed=0;

        if((*status2) & 0x1) sys_lepht_ctrl_pressed=1;
          else sys_lepht_ctrl_pressed=0;

        if((*status3) & 0x4) sys_right_ctrl_pressed=1;
          else sys_right_ctrl_pressed=0;
        }


int     sys_install_timer_isr(void)
        {
        if(sys_timer_isr_installed) return(0);
        else
          {
          sys_timer_isr_bios=_dos_getvect(0x8);
          _dos_setvect(0x8, sys_timer_isr);
          sys_timer_isr_installed=1;
          return(1);
          }
        }

int     sys_remove_timer_isr(void)
        {
        //if(!sys_timer_isr_installed) return(0);
        //else
        //  {
          _dos_setvect(0x8, sys_timer_isr_bios);
          sys_timer_isr_installed=0;
          return(1);
        //  }
        }
        
void    __interrupt __far sys_timer_isr()
        {
        sys_phrame_counter++;
        outp(0x20, 0x20);
        sys_timer_speed_add2+=sys_timer_speed;
        if(sys_timer_speed_add2>0xffff) sys_timer_speed_add2-=0xffff;
        _chain_intr(sys_timer_isr_bios);
        }
        
void    sys_set_timer_speed(unsigned short speed)
        {
        sys_timer_speed=(long)speed;
        outp(SYS_TIMER_CONTROL, SYS_TIMER_SET_CMD);
        outp(SYS_TIMER_0, BYTE_LO(speed));
        outp(SYS_TIMER_0, BYTE_HI(speed));
        }

