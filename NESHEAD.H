//旼컴컴컴컫컴컴컴쩡컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
//� Offset � Size � Content(s)                                           �
//쳐컴컴컴컵컴컴컴탠컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴캑
//�   0    �  3   � 'NES'                                                �
//�   3    �  1   � $1A (Control-Z)                                      �
//�   4    �  1   � Number of 16K PRG-ROM banks                          �
//�   5    �  1   � Number of 8K CHR-RAM banks                           �
//�   6    �  1   � ROM Control Byte #1                                  �
//�        �      �   %00000000                                          �
//�        �      �    읕쬔납냅� 0=Horizontal Mirroring                  �
//�        �      �      � 납�   1=Vertical Mirroring                    �
//�        �      �      � 납읕� 1=WRAM located at $6000-7FFF            �
//�        �      �      � 냅컴� 1=512-byte trainer present              �
//�        �      �      � 읕컴� 1=Four-screen VRAM layout               �
//�        �      �      �                                               �
//�        �      �      읕컴컴� iNES Mapper # (lower 4-bits)            �
//�   7    �  1   � ROM Control Byte #2                                  �
//�        �      �   %00000000                                          �
//�        �      �    읕쬔                                              �
//�        �      �      읕컴컴� iNES Mapper # (upper 4-bits)            �
//�        �      �                                                      �
//�  8-15  �  9   � [Reserved for expansion, should be 0]                �
//� 16-..  �      � PRG-ROM banks (in ascending order). A trainer pre-   �
//�        �      � cedes the first bank, if a trainer exists.           �
//� ..-EOF �      � CHR-RAM banks (in ascending order).                  �
//읕컴컴컴컨컴컴컴좔컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸

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

