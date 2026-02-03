/* Implementation of welcome screen functions */

#include <conio.h>

#include "galaksija.h"
#include "globals.h"
#include "welcome_screen.h"

/* Defines valid welcome screen states */
enum welcome_screen_state { WELCOME_TITLE, WELCOME_HELP, WELCOME_EXIT };

/* Current welcome screen state */
enum welcome_screen_state welcome_state = WELCOME_TITLE;


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
    gal_gotoxy(4, 6);
    gal_puts("< - DECREASE DT");
    gal_gotoxy(4, 7);
    gal_puts("> - INCREASE DT");
    gal_gotoxy(4, 8);
    gal_puts("1 - XY VIEW");
    gal_gotoxy(4, 9);
    gal_puts("2 - XZ VIEW");
    gal_gotoxy(4, 10);
    gal_puts("3 - YZ VIEW");
    gal_gotoxy(4, 11);
    gal_puts("S - TOGGLE STATS");

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
        if (welcome_state == WELCOME_TITLE)
        {
            welcome_state = WELCOME_HELP;
        }
        else if (welcome_state == WELCOME_HELP)
        {
            welcome_state = WELCOME_TITLE;
        }
        return;
    }
    else if (char_input == KEY_ENTER)
    {
        welcome_state = WELCOME_EXIT;
        return;
    }
    goto USER_INPUT;
}

void welcome_screen()
{
    while (welcome_state != WELCOME_EXIT)
    {
        switch (welcome_state)
        {
        case WELCOME_TITLE:
            draw_title_screen();
            break;
        case WELCOME_HELP:
            draw_help_screen();
            break;
        default:
            break;
        }

        handle_menu_user_input();

        clear_welcome_screen();
    }
}
