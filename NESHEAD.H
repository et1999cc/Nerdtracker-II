//ÚÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
//³ Offset ³ Size ³ Content(s)                                           ³
//ÃÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
//³   0    ³  3   ³ 'NES'                                                ³
//³   3    ³  1   ³ $1A (Control-Z)                                      ³
//³   4    ³  1   ³ Number of 16K PRG-ROM banks                          ³
//³   5    ³  1   ³ Number of 8K CHR-RAM banks                           ³
//³   6    ³  1   ³ ROM Control Byte #1                                  ³
//³        ³      ³   %00000000                                          ³
//³        ³      ³    ÀÄÂÙ³³³ÀÄ 0=Horizontal Mirroring                  ³
//³        ³      ³      ³ ³³³   1=Vertical Mirroring                    ³
//³        ³      ³      ³ ³³ÀÄÄ 1=WRAM located at $6000-7FFF            ³
//³        ³      ³      ³ ³ÀÄÄÄ 1=512-byte trainer present              ³
//³        ³      ³      ³ ÀÄÄÄÄ 1=Four-screen VRAM layout               ³
//³        ³      ³      ³                                               ³
//³        ³      ³      ÀÄÄÄÄÄÄ iNES Mapper # (lower 4-bits)            ³
//³   7    ³  1   ³ ROM Control Byte #2                                  ³
//³        ³      ³   %00000000                                          ³
//³        ³      ³    ÀÄÂÙ                                              ³
//³        ³      ³      ÀÄÄÄÄÄÄ iNES Mapper # (upper 4-bits)            ³
//³        ³      ³                                                      ³
//³  8-15  ³  9   ³ [Reserved for expansion, should be 0]                ³
//³ 16-..  ³      ³ PRG-ROM banks (in ascending order). A trainer pre-   ³
//³        ³      ³ cedes the first bank, if a trainer exists.           ³
//³ ..-EOF ³      ³ CHR-RAM banks (in ascending order).                  ³
//ÀÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

struct  nes_header_struc  {
        char            nt_nes_id[3];
        unsigned char   nt_ctrlz;
        unsigned char   nt_prgrom_tot;
        unsigned char   nt_chrrom_tot;
        unsigned char   nt_romctrlbyte1;
        unsigned char   nt_romctrlbyte2;
        unsigned char   nt_reserved[8];
        };

struct  nes_header_struc        nes_header = {
        'N','E','S',
        0x1a,
        2, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0};

