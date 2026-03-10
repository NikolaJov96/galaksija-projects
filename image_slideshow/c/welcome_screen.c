/* Welcome screen for the image slideshow */

#include <conio.h>

#include "galaksija.h"
#include "welcome_screen.h"

#define SCREEN_WIDTH  32
#define SCREEN_HEIGHT 16
#define KEY_ENTER     10

/*
 * Block character values for a 1-pixel border.
 * Each character encodes a 2x3 pixel block; bit layout:
 *   bit 0 ( 1): top-left     bit 1 ( 2): top-right
 *   bit 2 ( 4): mid-left     bit 3 ( 8): mid-right
 *   bit 4 (16): bot-left     bit 5 (32): bot-right
 *
 * Top edge    : bits 0+1        = 128+3   = 131
 * Bottom edge : bits 4+5        = 128+48  = 176
 * Left edge   : bits 0+2+4      = 128+21  = 149
 * Right edge  : bits 1+3+5      = 128+42  = 170
 * Corner TL   : bits 0+1+2+4    = 128+23  = 151
 * Corner TR   : bits 0+1+3+5    = 128+43  = 171
 * Corner BL   : bits 0+2+4+5    = 128+53  = 181
 * Corner BR   : bits 1+3+4+5    = 128+58  = 186
 */
void draw_border()
{
    int i;

    for (i = 1; i < SCREEN_WIDTH - 1; i++)
    {
        gal_gotoxy(i, 0);
        gal_putc(131);
        gal_gotoxy(i, SCREEN_HEIGHT - 1);
        gal_putc(176);
    }

    for (i = 1; i < SCREEN_HEIGHT - 1; i++)
    {
        gal_gotoxy(0, i);
        gal_putc(149);
        gal_gotoxy(SCREEN_WIDTH - 1, i);
        gal_putc(170);
    }

    gal_gotoxy(0, 0);                         gal_putc(151); /* TL */
    gal_gotoxy(SCREEN_WIDTH - 1, 0);          gal_putc(171); /* TR */
    gal_gotoxy(0, SCREEN_HEIGHT - 1);         gal_putc(181); /* BL */
    gal_gotoxy(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1); gal_putc(186); /* BR */
}

void welcome_screen()
{
    draw_border();

    gal_gotoxy(6, 2);
    gal_puts("********************");
    gal_gotoxy(6, 3);
    gal_puts("*  IMAGE SLIDESHOW *");
    gal_gotoxy(6, 4);
    gal_puts("********************");

    gal_gotoxy(8, 6);
    gal_puts("NIKOLA JOVANOVI");
    gal_putc(92);
    gal_gotoxy(13, 7);
    gal_puts("2026");

    gal_gotoxy(4, 9);
    gal_puts("SQUINT FOR BEST RESULTS");

    gal_gotoxy(4, 11);
    gal_puts("< - PREVIOUS");
    gal_gotoxy(4, 12);
    gal_puts("> - NEXT");
    gal_gotoxy(4, 13);
    gal_puts("DEL - EXIT");

    gal_gotoxy(18, SCREEN_HEIGHT - 2);
    gal_puts("ENTER - START");

WAIT_FOR_ENTER:
    if (fgetc_cons() != KEY_ENTER)
        goto WAIT_FOR_ENTER;
}
