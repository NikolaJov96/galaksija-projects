#include <stdint.h>
#include <conio.h>

#include "galaksija.h"

#define SCREEN_WIDTH 32
#define SCREEN_WIDTH_HALF 16
#define SCREEN_HEIGHT 16
#define SCREEN_HEIGHT_HALF 8

#define KEY_ENTER 10
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_S 83

#define PATH_LENGTH_SHIFT 5
/* Length of the path history array.
   Chosen to be a power of two so the modulo operation
   can be optimized to a bitwise AND.*/
#define PATH_LENGTH (1l << PATH_LENGTH_SHIFT)

#define IGNORE_BUTTON_COOLDOWN 15

// Fixed point arithmetic macros

/* Number of simulated decimal places.
   This number should be as large as possible,
   but such that it never causes overflow */
#define FP_SHIFT 8
#define TO_FIXED(x) ((int32_t)((x) * (1l << FP_SHIFT)))
#define FROM_FIXED(x) ((x) >> FP_SHIFT)
/* Multiplication of two fixed point numbers results in a double
   shifted value, so it needs to be shifted one step back */
#define FIXED_MUL(a, b) (FROM_FIXED(((a) * (b))))

// TODO:
// - Add visit count to path history so the pixel deletion is correct
// - Use different chars for path history fading
// - Use dot instead of char resolution

/* Marks the camera view angle of the system */
enum view_axis {
    ASIS_XY, ASIS_XZ, ASIS_YZ
};

/* Marks whether statistics are visible */
enum stats_visibility {
    STATS_OFF, STATS_ON
};

/* Should path reset erase the path history pixels */
enum reset_erase_mode {
    NO_ERASE, ERASE
};

// Path history tracking variables

/* History of the last PATH_LENGTH X screen coordinates */
int positions_x[PATH_LENGTH];
/* History of the last PATH_LENGTH Y screen coordinates */
int positions_y[PATH_LENGTH];
/* Index of the current position in revolving path history arrays */
int path_index = 0;
/* Helper variable containing the index of the oldest position in path history */
int oldest_path_index = 0;

// Display stats

/* Stats visiblity flag */
enum stats_visibility print_stats = STATS_OFF;
/* Cooldown required between registering button presses */
unsigned char ignore_button_cooldown = 0;
/* Latest pressed button character */
unsigned char char_input;
/* Simulation iteration counter */
uint32_t iteration = 1;

// Lorenz attractor parameters
// Parameters are hardcoded to values that produce a chaotic behavior

/* The first of the Lorenz attractor parameters */
int32_t ro = TO_FIXED(28.0);
/* The second of the Lorenz attractor parameters */
int32_t sigma = TO_FIXED(10.0);
/* The third of the Lorenz attractor parameters */
int32_t beta = TO_FIXED(8.0 / 3.0);
/* Simulation time step */
int32_t dt = TO_FIXED(0.01);

// System state variables

/* The X coordinate of the system */
int32_t x = TO_FIXED(1.0);
/* The Y coordinate of the system */
int32_t y = TO_FIXED(1.0);
/* The Z coordinate of the system */
int32_t z = TO_FIXED(1.0);
/* Current camera projection */
enum view_axis projection = ASIS_XZ;

// Simulation helper variables

/* Change in X coordinate in the current step */
int32_t dx = 0;
/* Change in Y coordinate in the current step */
int32_t dy = 0;
/* Change in Z coordinate in the current step */
int32_t dz = 0;
/* Screen X coordinate of the system */
int screen_x = SCREEN_WIDTH_HALF;
/* Screen Y coordinate of the system */
int screen_y = SCREEN_HEIGHT_HALF;

/* Print welcome screen */
void print_welcome_screen()
{
    int i;

    // Welcome screen
    gal_cls();

    // Vertical border
    for (i = 0; i < SCREEN_HEIGHT - 0; i++) {
        gal_gotoxy(0, i);
        gal_putc('*');
        gal_gotoxy(SCREEN_WIDTH - 1, i);
        gal_putc('*');
    }

    // Horizontal border
    for (i = 1; i < SCREEN_WIDTH - 1; i++) {
        gal_gotoxy(i, 0);
        gal_putc('*');
        gal_gotoxy(i, SCREEN_HEIGHT - 1);
        gal_putc('*');
    }

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
}

/* Clears the welcome screen */
void clear_welcome_screen()
{
    for (int i = 1; i < SCREEN_HEIGHT - 1; i++) {
        gal_gotoxy(1, i);
        gal_puts("                              ");
    }
}

/* Prints an int value to the screen, location should already be set.
  Adds minus if negative, adds spaces if less than 3 digits. */
void print_int_3(int value)
{
    int max_value = 99;
    int digit = 0;
    int i = 0;

    if (value < 0) {
        gal_putc('-');
        i++;
        value = -value;
    }

    if (value > max_value)
    {
        value = max_value;
    }

    if (value >= 10)
    {
        digit = value / 10;
        gal_putc('0' + digit);
        i++;
        value -= digit * 10;
    }

    // Value is now a single digit
    gal_putc('0' + value);
    i++;

    // Pad with spaces if needed
    while (i < 3) {
        gal_putc(' ');
        i++;
    }
}

/* Prints a positive long int value to screen with max 6 digits,
  location should already be set. */
void print_int_6(int32_t value)
{
    int32_t max_value = 999999;
    int digit = 0;
    int i = 0;

    if (value > max_value)
    {
        value = max_value;
    }
    max_value++;

    while (max_value > value)
    {
        max_value /= 10;
    }

    while (max_value >= 10)
    {
        if (value >= max_value)
        {
            digit = value / max_value;
            gal_putc('0' + digit);
            i++;
            value -= digit * max_value;
        }
        else
        {
            gal_putc('0');
            i++;
        }
        max_value /= 10;
    }

    // Value is now a single digit
    gal_putc('0' + (char)value);
    i++;
}

/* Handles initialization and cleaning of stats display */
void toggle_stats()
{
    if (print_stats == STATS_OFF) {
        print_stats = STATS_ON;
        gal_gotoxy(1, SCREEN_HEIGHT - 5);
        gal_puts("X: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 4);
        gal_puts("Y: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 3);
        gal_puts("Z: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 2);
        gal_puts("ITER: ");
    } else {
        print_stats = STATS_OFF;
        gal_gotoxy(1, SCREEN_HEIGHT - 5);
        gal_puts("         ");
        gal_gotoxy(1, SCREEN_HEIGHT - 4);
        gal_puts("         ");
        gal_gotoxy(1, SCREEN_HEIGHT - 3);
        gal_puts("         ");
        gal_gotoxy(1, SCREEN_HEIGHT - 2);
        gal_puts("            ");
    }
}

/* Resets the path history arrays to -1. Optionally, erases the currently drawn points.
   Erasing should be disabled when called for the first time
   because the arrays are yet to be initialized to -1. */
void reinitialize_path_history(enum reset_erase_mode erase_mode)
{
    for (int i = 0; i < PATH_LENGTH; i++)
    {
        if (erase_mode == ERASE && positions_x[i] != -1 && positions_y[i] != -1)
        {
            gal_gotoxy(positions_x[i], positions_y[i]);
            gal_putc(' ');
        }

        positions_x[i] = -1;
        positions_y[i] = -1;
    }
    path_index = 0;
}

/* Handles user input actions */
void handle_user_input()
{
    if (ignore_button_cooldown == 0)
    {
        char_input = getk();
        switch (char_input)
        {
        case KEY_1:
            if (projection != ASIS_XY)
            {
                projection = ASIS_XY;
                reinitialize_path_history(ERASE);
            }
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case KEY_2:
            if (projection != ASIS_XZ)
            {
                projection = ASIS_XZ;
                reinitialize_path_history(ERASE);
            }
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case KEY_3:
            if (projection != ASIS_YZ)
            {
                projection = ASIS_YZ;
                reinitialize_path_history(ERASE);
            }
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case KEY_S:
            toggle_stats();
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        default:
            break;
        }
    }
    else
    {
        ignore_button_cooldown--;
    }
}

int main()
{
    print_welcome_screen();
    while (fgetc_cons() != KEY_ENTER);
    clear_welcome_screen();

    reinitialize_path_history(NO_ERASE);

SIM_ITER:
    // Compute derivatives
    dx = FIXED_MUL(sigma, (y - x));
    dy = FIXED_MUL(x, (ro - z)) - y;
    dz = FIXED_MUL(x, y) - FIXED_MUL(beta, z);

    // Update state
    x += FIXED_MUL(dx, dt);
    y += FIXED_MUL(dy, dt);
    z += FIXED_MUL(dz, dt);

    // Map state to screen coordinates
    switch (projection)
    {
        case ASIS_XY:
            screen_x = SCREEN_WIDTH_HALF + FROM_FIXED(x) / 2;
            screen_y = SCREEN_HEIGHT_HALF - FROM_FIXED(y) / 2;
            break;
        case ASIS_XZ:
            screen_x = SCREEN_WIDTH_HALF + FROM_FIXED(x) / 2;
            screen_y = SCREEN_HEIGHT_HALF - (FROM_FIXED(z) - 25) / 2;
            break;
        case ASIS_YZ:
            screen_x = SCREEN_WIDTH_HALF + FROM_FIXED(y) / 2;
            screen_y = SCREEN_HEIGHT_HALF - (FROM_FIXED(z) - 25) / 2;
            break;
    }

    // Update the screen and path history if the point is within screen bounds
    // and has moved since the last iteration
    if (screen_x >= 1 && screen_x < SCREEN_WIDTH - 1 &&
        screen_y >= 1 && screen_y < SCREEN_HEIGHT - 1 &&
        (screen_x != positions_x[path_index] || screen_y != positions_y[path_index]))
    {
        // Erase oldest point in path history
        oldest_path_index = (path_index + 1) & (PATH_LENGTH - 1);
        if (positions_x[oldest_path_index] != -1 && positions_y[oldest_path_index] != -1)
        {
            gal_gotoxy(positions_x[oldest_path_index], positions_y[oldest_path_index]);
            gal_putc(' ');
        }

        // Move to the next position in the path history
        path_index = oldest_path_index;

        // Store new position in history
        positions_x[path_index] = screen_x;
        positions_y[path_index] = screen_y;

        gal_gotoxy(screen_x, screen_y);
        gal_putc('#');
    }

    // Update stats display if enabled
    if (print_stats == STATS_ON)
    {
        gal_gotoxy(4, SCREEN_HEIGHT - 5);
        print_int_3(FROM_FIXED((int)x));

        gal_gotoxy(4, SCREEN_HEIGHT - 4);
        print_int_3(FROM_FIXED((int)y));

        gal_gotoxy(4, SCREEN_HEIGHT - 3);
        print_int_3(FROM_FIXED((int)z));

        gal_gotoxy(7, SCREEN_HEIGHT - 2);
        print_int_6((int)iteration);
    }

    handle_user_input();

    iteration++;
    goto SIM_ITER;

    return 0;
}
