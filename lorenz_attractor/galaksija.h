/* Header file with specific GALAKSIJA low-level functions */

#ifndef GALAKSIJA_H
#define GALAKSIJA_H

#define z80_bpoke(a,b)  (*(unsigned char *)(a) = b)
#define z80_wpoke(a,b)  (*(unsigned int *)(a) = b)
#define z80_lpoke(a,b)  (*(unsigned long *)(a) = b)
#define z80_bpeek(a)    (*(unsigned char *)(a))
#define z80_wpeek(a)    (*(unsigned int *)(a))
#define z80_lpeek(a)    (*(unsigned long *)(a))

#define SCREEN_ADDR 0x2800
#define RND_ADDR 0x2AA7

/* Low-level clear the screen and reset internal cursor position */
void gal_cls();

 /* Set internal position of cursor for low-level printing */
void gal_gotoxy(char x, char y);

 /* Low-level write character to internal cursor position */
void gal_putc(char ch);

 /* Low-level write string to internal cursor position */
int gal_puts(char *str);

#endif // GALAKSIJA_H
