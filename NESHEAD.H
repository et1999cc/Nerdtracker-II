//����������������������������������������������������������������������Ŀ
//� Offset � Size � Content(s)                                           �
//����������������������������������������������������������������������Ĵ
//�   0    �  3   � 'NES'                                                �
//�   3    �  1   � $1A (Control-Z)                                      �
//�   4    �  1   � Number of 16K PRG-ROM banks                          �
//�   5    �  1   � Number of 8K CHR-RAM banks                           �
//�   6    �  1   � ROM Control Byte #1                                  �
//�        �      �   %00000000                                          �
//�        �      �    ���ٳ���� 0=Horizontal Mirroring                  �
//�        �      �      � ���   1=Vertical Mirroring                    �
//�        �      �      � ����� 1=WRAM located at $6000-7FFF            �
//�        �      �      � ����� 1=512-byte trainer present              �
//�        �      �      � ����� 1=Four-screen VRAM layout               �
//�        �      �      �                                               �
//�        �      �      ������� iNES Mapper # (lower 4-bits)            �
//�   7    �  1   � ROM Control Byte #2                                  �
//�        �      �   %00000000                                          �
//�        �      �    ����                                              �
//�        �      �      ������� iNES Mapper # (upper 4-bits)            �
//�        �      �                                                      �
//�  8-15  �  9   � [Reserved for expansion, should be 0]                �
//� 16-..  �      � PRG-ROM banks (in ascending order). A trainer pre-   �
//�        �      � cedes the first bank, if a trainer exists.           �
//� ..-EOF �      � CHR-RAM banks (in ascending order).                  �
//������������������������������������������������������������������������

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
