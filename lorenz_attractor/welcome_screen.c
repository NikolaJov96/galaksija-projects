/* Implementation of welcome screen functions */

#include <conio.h>

#include "galaksija.h"
#include "globals.h"
#include "welcome_screen.h"


/* Clears the screen, except the border pixels */
void clear_welcome_screen()
{
    for (int i = 1; i < SCREEN_HEIGHT - 1; i++)
    {
        gal_gotoxy(1, i);
        gal_puts("                              ");
    }
}

/* Draws the title screen */
void draw_title_screen()
{
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 4);
    gal_puts("********************");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 3);
    gal_puts("* LORENZ ATTRACTOR *");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 2);
    gal_puts("********************");

    gal_gotoxy(6, SCREEN_HEIGHT_HALF);
    gal_puts("BY NIKOLA JOVANOVI");
    gal_putc(92);

    gal_gotoxy(6, SCREEN_HEIGHT_HALF + 3);
    gal_puts("H - HELP SCREEN");

    gal_gotoxy(20, SCREEN_HEIGHT - 2);
    gal_puts("PRESS ENTER");
}

/* Draws the help screen */
void draw_help_screen()
{
    // Description
    gal_gotoxy(4, 2);
    gal_puts("WELCOME TO THE LORENZ");
    gal_gotoxy(4, 3);
    gal_puts("ATTRACTOR SIMULATION!");

    // Commands
    gal_gotoxy(4, 5);
    gal_puts("COMMANDS:");
    gal_gotoxy(6, 6);
    gal_puts("< - DECREASE DT");
    gal_gotoxy(6, 7);
    gal_puts("> - INCREASE DT");
    gal_gotoxy(6, 8);
    gal_puts("1 - XY VIEW");
    gal_gotoxy(6, 9);
    gal_puts("2 - XZ VIEW");
    gal_gotoxy(6, 10);
    gal_puts("3 - YZ VIEW");
    gal_gotoxy(6, 11);
    gal_puts("S - TOGGLE STATS");
    gal_gotoxy(4, 12);
    gal_puts("DEL - QUIT SIMULATION");

    gal_gotoxy(20, SCREEN_HEIGHT - 2);
    gal_puts("PRESS ENTER");
}

/* Waits for user input to update the welcome screen state */
void handle_menu_user_input()
{
USER_INPUT:
    char_input = fgetc_cons();
    if (char_input == 'H')
    {
        if (program_state == TITLE_SCREEN)
        {
            program_state = HELP_SCREEN;
        }
        else if (program_state == HELP_SCREEN)
        {
            program_state = TITLE_SCREEN;
        }
        return;
    }
    else if (char_input == KEY_ENTER)
    {
        program_state = SIMULATION;
        return;
    }
    goto USER_INPUT;
}

void welcome_screen()
{
    while (program_state != SIMULATION)
    {
        switch (program_state)
        {
        case TITLE_SCREEN:
            draw_title_screen();
            break;
        case HELP_SCREEN:
            draw_help_screen();
            break;
        default:
            break;
        }

        handle_menu_user_input();

        clear_welcome_screen();
    }
}
