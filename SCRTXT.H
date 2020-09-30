
        // Variables

extern  unsigned char   *scrtxt_vidbuphph;
extern  unsigned char   *scrtxt_vidmem;
extern  int             scrtxt_ScreenWidth;
extern  int             scrtxt_ScreenHeight;

extern  char    cursor_blink;


        // Functions

// Sets high-resolution textmode
        void    scrtxt_set_hires(void);

        void    scrtxt_set_blink(int Mode);

// Sets low-resolution textmode
        void    scrtxt_set_lores(void);

// Writes a character to the speciphied position with speciphied attributes
        void    scrtxt_write_char(int x, int y, char the_char, char color, char bcolor, char blink);
        
// Writes a character to the speciphied position with speciphied color
        void    scrtxt_write_col(int x, int y, char color, char blink);


// Copies *pic to the screen
        void    scrtxt_put_pic(void *pic);

// Copies  specified number of lines of *pic to the screen 
        void    scrtxt_put_pic_lines(void *pic, int lines);

// Copies specified number of lines of *pic to the screen and clears the
// Remaining lines
        void    scrtxt_put_pic_lines_clear(void *pic, int lines);

// Waits phor a vertical retrace
        void    scrall_wait_vr(void);

// Sets the position oph the cursor
        void    scrtxt_set_cursor_pos(int x, int y);

        void    scrtxt_copy2vidmem(void);



