/* Implementation of welcome screen functions */

#include <conio.h>

#include "galaksija.h"
#include "globals.h"
#include "welcome_screen.h"

void welcome_screen()
{
    int i;

    // Title
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 4);
    gal_puts("********************");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 3);
    gal_puts("* LORENZ ATTRACTOR *");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 2);
    gal_puts("********************");

    gal_gotoxy(6, SCREEN_HEIGHT_HALF);
    gal_puts("BY NIKOLA JOVANOVI");
    gal_putc(92);

    // Commands
    gal_gotoxy(2, SCREEN_HEIGHT_HALF + 2);
    gal_puts("1 - XY VIEW");
    gal_gotoxy(2, SCREEN_HEIGHT_HALF + 3);
    gal_puts("2 - XZ VIEW");
    gal_gotoxy(2, SCREEN_HEIGHT_HALF + 4);
    gal_puts("3 - YZ VIEW");
    gal_gotoxy(2, SCREEN_HEIGHT_HALF + 5);
    gal_puts("S - TOGGLE STATS");

    // Enter prompt
    gal_gotoxy(20, SCREEN_HEIGHT - 2);
    gal_puts("PRESS ENTER");

    while (fgetc_cons() != KEY_ENTER);

    // Clear the welcome screen
    for (int i = 1; i < SCREEN_HEIGHT - 1; i++)
    {
        gal_gotoxy(1, i);
        gal_puts("                              ");
    }
}
