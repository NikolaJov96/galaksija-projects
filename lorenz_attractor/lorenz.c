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
#define PATH_LENGTH (1l << PATH_LENGTH_SHIFT)

#define FP_SHIFT 8
#define TO_FIXED(x) ((int32_t)((x) * (1l << FP_SHIFT)))
#define FROM_FIXED(x) ((x) >> FP_SHIFT)
#define FIXED_MUL(a, b) (FROM_FIXED(((a) * (b))))

// TODO:
// - Better formating of stats display
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

// Path history variables, globals to preserve stack space
int positions_x[PATH_LENGTH];
int positions_y[PATH_LENGTH];
int path_index = 0;
int oldest_path_index = 0;

// Display labels
enum stats_visibility print_labels = STATS_OFF;
int ignore_button_ticks = 0;
uint32_t iteration = 1;

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

void reset_path_history(enum reset_erase_mode erase_mode)
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

int main()
{
    print_welcome_screen();

    // Wait for Enter key
    unsigned char char_input;
    do {
        char_input = fgetc_cons();
    } while (char_input != KEY_ENTER);

    clear_welcome_screen();

    // Lorenz attractor parameters
    int32_t ro = TO_FIXED(28.0);
    int32_t sigma = TO_FIXED(10.0);
    int32_t beta = TO_FIXED(8.0 / 3.0);

    // Initial conditions
    int32_t x = TO_FIXED(1.0);
    int32_t y = TO_FIXED(1.0);
    int32_t z = TO_FIXED(1.0);

    // Path history without erasing, because the arrays are uninitialized
    reset_path_history(NO_ERASE);

    // Time step
    int32_t dt = TO_FIXED(0.01);

    // Projection axis
    enum view_axis projection = ASIS_XZ;

    // Loop variable initialization
    int32_t dx = 0;
    int32_t dy = 0;
    int32_t dz = 0;
    int screen_x = SCREEN_WIDTH_HALF;
    int screen_y = SCREEN_HEIGHT_HALF;

ITER:
    // Compute derivatives
    dx = FIXED_MUL(sigma, (y - x));
    dy = FIXED_MUL(x, (ro - z)) - y;
    dz = FIXED_MUL(x, y) - FIXED_MUL(beta, z);

    // Update state
    x += FIXED_MUL(dx, dt);
    y += FIXED_MUL(dy, dt);
    z += FIXED_MUL(dz, dt);

    // Map to screen coordinates
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

    // Plot point if within bounds
    if (screen_x >= 1 && screen_x < SCREEN_WIDTH - 1 &&
        screen_y >= 1 && screen_y < SCREEN_HEIGHT - 1)
    {
        if (screen_x != positions_x[path_index] || screen_y != positions_y[path_index])
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
        }

        gal_gotoxy(screen_x, screen_y);
        gal_putc('#');
    }

    // Update iteration count
    if (print_labels == STATS_ON)
    {
        gal_gotoxy(4, SCREEN_HEIGHT - 4);
        print_int_3(FROM_FIXED((int)x));

        gal_gotoxy(4, SCREEN_HEIGHT - 3);
        print_int_3(FROM_FIXED((int)y));

        gal_gotoxy(4, SCREEN_HEIGHT - 2);
        print_int_3(FROM_FIXED((int)z));

        gal_gotoxy(7, SCREEN_HEIGHT - 1);
        print_int_6((int)iteration);
    }

    if (ignore_button_ticks == 0)
    {
        char_input = getk();
        switch (char_input)
        {
        case KEY_1:
            if (projection != ASIS_XY)
            {
                projection = ASIS_XY;
                reset_path_history(ERASE);
                ignore_button_ticks = 15;
            }
            break;
        case KEY_2:
            if (projection != ASIS_XZ)
            {
                projection = ASIS_XZ;
                reset_path_history(ERASE);
                ignore_button_ticks = 15;
            }
            break;
        case KEY_3:

            if (projection != ASIS_YZ)
            {
                projection = ASIS_YZ;
                reset_path_history(ERASE);
                ignore_button_ticks = 15;
            }
            break;
            break;
        case KEY_S:
            if (print_labels == STATS_OFF) {
                print_labels = STATS_ON;
                gal_gotoxy(1, SCREEN_HEIGHT - 4);
                gal_puts("X: ");
                gal_gotoxy(1, SCREEN_HEIGHT - 3);
                gal_puts("Y: ");
                gal_gotoxy(1, SCREEN_HEIGHT - 2);
                gal_puts("Z: ");
                gal_gotoxy(1, SCREEN_HEIGHT - 1);
                gal_puts("ITER: ");
                ignore_button_ticks = 15;
            } else {
                print_labels = STATS_OFF;
                gal_gotoxy(1, SCREEN_HEIGHT - 4);
                gal_puts("         ");
                gal_gotoxy(1, SCREEN_HEIGHT - 3);
                gal_puts("         ");
                gal_gotoxy(1, SCREEN_HEIGHT - 2);
                gal_puts("         ");
                gal_gotoxy(1, SCREEN_HEIGHT - 1);
                gal_puts("*************");
                ignore_button_ticks = 15;
            }
            break;
        default:
            break;
        }
    }

    if (ignore_button_ticks > 0)
    {
        ignore_button_ticks--;
    }
    iteration++;
    goto ITER;

    return 0;
}
