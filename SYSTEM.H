#define SYS_TIMER_SPEED_120HZ           0x26d7
#define SYS_TIMER_SPEED_60HZ            0x4dae
#define SYS_TIMER_SPEED_50HZ            0x5d37
#define SYS_TIMER_SPEED_18HZ            0xffff

#define SYS_TIMER_CONTROL               0x43    // The 8253's control reg
#define SYS_TIMER_SET_CMD               0x3c    // The set speed command

#define SYS_TIMER_0                     0x40
#define SYS_TIMER_1                     0x41
#define SYS_TIMER_2                     0x42


extern  int     sys_timer_isr_installed;
extern  long    sys_phrame_counter;
extern  long    sys_timer_speed_add2;
extern  long    sys_timer_speed;

extern  int     sys_lepht_shipht_pressed;
extern  int     sys_right_shipht_pressed;
extern  int     sys_lepht_ctrl_pressed;
extern  int     sys_right_ctrl_pressed;
extern  int     sys_lepht_alt_pressed;
extern  int     sys_right_alt_pressed;


void    sys_do_misc_keys(void);

//void    (__interrupt __far *sys_bios_timer_isr)();

int     sys_install_timer_isr(void);

int     sys_remove_timer_isr(void);

void    __interrupt __far sys_timer_isr();
        
void    sys_set_timer_speed(unsigned short speed);

