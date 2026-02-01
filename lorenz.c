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

#define FP_SHIFT 8
#define TO_FIXED(x) ((int32_t)((x) * (1l << FP_SHIFT)))
#define FROM_FIXED(x) ((x) >> FP_SHIFT)
#define FIXED_MUL(a, b) (FROM_FIXED(((a) * (b))))

/* Marks the camera view angle of the system */
enum view_axis {
    ASIS_XY, ASIS_XZ, ASIS_YZ
};

/* Marks whether statistics are visible */
enum stats_visibility {
    STATS_OFF, STATS_ON
};

/* Prints an int value into a string, adds minus if negative, adds spaces if less than 3 digits. */
void print_int_3(char* str, int value)
{
    int max_value = 99;
    int digit = 0;
    int i = 0;

    if (value < 0) {
        str[i++] = '-';
        value = -value;
    }

    if (value > max_value)
    {
        value = max_value;
    }

    if (value >= 10)
    {
        digit = value / 10;
        str[i++] = '0' + digit;
        value -= digit * 10;
    }

    // Value is now a single digit
    str[i++] = '0' + value;

    // Pad with spaces if needed
    while (i < 3) {
        str[i++] = ' ';
    }

    // Null-terminate
    str[i++] = 0;
}

/* Prints a positive long int value into a string. */
void print_int_6(char* str, int32_t value)
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
            str[i++] = '0' + digit;
            value -= digit * max_value;
        }
        else
        {
            str[i++] = '0';
        }
        max_value /= 10;
    }

    // Value is now a single digit
    str[i++] = '0' + value;

    // Null-terminate
    str[i++] = 0;
}

int main()
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
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 2);
    gal_puts("********************");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 1);
    gal_puts("* LORENZ ATTRACTOR *");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF);
    gal_puts("********************");

    // Enter prompt
    gal_gotoxy(20, SCREEN_HEIGHT - 2);
    gal_puts("PRESS ENTER");

    // Wait for Enter key
    unsigned char input;
    do {
        input = fgetc_cons();
    } while (input != KEY_ENTER);

    // Remove title and prompt
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 2);
    gal_puts("                    ");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF - 1);
    gal_puts("                    ");
    gal_gotoxy(6, SCREEN_HEIGHT_HALF);
    gal_puts("                    ");
    gal_gotoxy(20, SCREEN_HEIGHT - 2);
    gal_puts("           ");

    // Lorenz attractor parameters
    int32_t ro = TO_FIXED(28.0);
    int32_t sigma = TO_FIXED(10.0);
    int32_t beta = TO_FIXED(8.0 / 3.0);

    // Initial conditions
    int32_t x = TO_FIXED(1.0);
    int32_t y = TO_FIXED(1.0);
    int32_t z = TO_FIXED(1.0);

    // Time step
    int32_t dt = TO_FIXED(0.01);

    // Projection axis
    enum view_axis projection = ASIS_XY;

    // Display labels
    enum stats_visibility print_labels = STATS_OFF;
    uint32_t iteration = 1;
    char num_str[32];

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

    // Remove previous point
    if (screen_x >= 1 && screen_x < SCREEN_WIDTH - 1 &&
        screen_y >= 1 && screen_y < SCREEN_HEIGHT - 1) {
        gal_gotoxy(screen_x, screen_y);
        gal_putc(' ');
    }

    // Map to screen coordinates
    switch (projection)
    {
        case ASIS_XY:
            screen_x = SCREEN_WIDTH_HALF + FROM_FIXED(x) / 2;
            screen_y = SCREEN_HEIGHT_HALF + FROM_FIXED(y) / 2;
            break;
        case ASIS_XZ:
            screen_x = SCREEN_WIDTH_HALF + FROM_FIXED(x) / 2;
            screen_y = SCREEN_HEIGHT_HALF + (FROM_FIXED(z) - 25) / 2;
            break;
        case ASIS_YZ:
            screen_x = SCREEN_WIDTH_HALF + FROM_FIXED(y) / 2;
            screen_y = SCREEN_HEIGHT_HALF + (FROM_FIXED(z) - 25) / 2;
            break;
    }

    // Plot point if within bounds
    if (screen_x >= 1 && screen_x < SCREEN_WIDTH - 1 &&
        screen_y >= 1 && screen_y < SCREEN_HEIGHT - 1)
    {
        gal_gotoxy(screen_x, screen_y);
        gal_putc('#');
    }

    // Update iteration count
    if (print_labels == STATS_ON)
    {
        gal_gotoxy(4, SCREEN_HEIGHT - 4);
        print_int_3(num_str, FROM_FIXED((int)x));
        gal_puts(num_str);

        gal_gotoxy(4, SCREEN_HEIGHT - 3);
        print_int_3(num_str, FROM_FIXED((int)y));
        gal_puts(num_str);

        gal_gotoxy(4, SCREEN_HEIGHT - 2);
        print_int_3(num_str, FROM_FIXED((int)z));
        gal_puts(num_str);

        gal_gotoxy(7, SCREEN_HEIGHT - 1);
        print_int_6(num_str, (int)iteration);
        gal_puts(num_str);
    }

    input = getk();
    switch (input)
    {
    case KEY_1:
        projection = ASIS_XY;
        break;
    case KEY_2:
        projection = ASIS_XZ;
        break;
    case KEY_3:
        projection = ASIS_YZ;
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
        }
        break;
    default:
        break;
    }

    iteration++;
    goto ITER;

    return 0;
}
