/* Main logic of the Lorenz attractor simulation */

#include <conio.h>

#include "galaksija.h"
#include "globals.h"
#include "welcome_screen.h"

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
    else
    {
        gal_putc(' ');
        i++;
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

/* Prints a positive int value to screen with max 4 digits,
  location should already be set. */
void print_positive_int(int value)
{
    int max_value = 9999;
    int digit = 0;

    if (value < 0 || value > max_value)
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
            value -= digit * max_value;
        }
        else
        {
            gal_putc('0');
        }
        max_value /= 10;
    }

    // Value is now a single digit
    gal_putc('0' + (char)value);
}

/* Handles initialization and cleaning of stats display */
void toggle_stats()
{
    if (print_stats == STATS_OFF) {
        print_stats = STATS_ON;
        gal_gotoxy(1, SCREEN_HEIGHT - 6);
        gal_puts("X: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 5);
        gal_puts("Y: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 4);
        gal_puts("Z: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 3);
        gal_puts("DT: ");
        gal_gotoxy(1, SCREEN_HEIGHT - 2);
        gal_puts("ITER: ");
    } else {
        print_stats = STATS_OFF;
        gal_gotoxy(1, SCREEN_HEIGHT - 6);
        gal_puts("         ");
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

/* Updates the screen character at the screen_x, screen_y position.
   Checks the the appropriate 6 pixels for the visit count array,
   calculates the appropriate character, and updates the screen */
void update_screen_char()
{
    const unsigned char x = screen_x * 2;
    const unsigned char y = screen_y * 3;

    char_input = 128;
    char_input += (visit_count[y][x] > 0) * 1;
    char_input += (visit_count[y][x + 1] > 0) * 2;
    char_input += (visit_count[y + 1][x] > 0) * 4;
    char_input += (visit_count[y + 1][x + 1] > 0) * 8;
    char_input += (visit_count[y + 2][x] > 0) * 16;
    char_input += (visit_count[y + 2][x + 1] > 0) * 32;

    gal_gotoxy(screen_x, screen_y);
    gal_putc(char_input);
}

/* Resets the path history arrays to -1. Optionally, erases the currently drawn points.
   Erasing should be disabled when called for the first time
   because the arrays are yet to be initialized to -1. */
void reinitialize_path_history(enum call_mode call_mode)
{
    int i, j;

    // First, reset visit counts to make sure the screen update
    // will work correctly in the next loop.
    for (i = 1; i < GRID_HEIGHT - 1; i++)
    {
        for (j = 1; j < GRID_WIDTH - 1; j++)
        {
            visit_count[i][j] = 0;
        }
    }

    // Reset the position arrays and update the screen
    for (i = 0; i < PATH_LENGTH; i++)
    {
        if (call_mode == CALL_RUN &&
            positions_x[i] > 1 && positions_x[i] < GRID_WIDTH - 1 &&
            positions_y[i] > 1 && positions_y[i] < GRID_HEIGHT - 1)
        {
            screen_x = positions_x[i] / 2;
            screen_y = positions_y[i] / 3;
            update_screen_char();
        }

        positions_x[i] = -1;
        positions_y[i] = -1;
    }

    // First call to this function, initialize the border cells.
    // These will remain untouched throughout the simulation.
    if (call_mode == CALL_INIT)
    {
        // Initialize border grid cells
        for (i = 0; i < GRID_HEIGHT; i++)
        {
            visit_count[i][0] = 1;
            visit_count[i][GRID_WIDTH - 1] = 1;
        }
        for (i = 0; i < GRID_WIDTH; i++)
        {
            visit_count[0][i] = 1;
            visit_count[GRID_HEIGHT - 1][i] = 1;
        }

        // Draw border pixels
        for (i = 0; i < SCREEN_HEIGHT; i++)
        {
            screen_x = 0;
            screen_y = i;
            update_screen_char();
            screen_x = SCREEN_WIDTH - 1;
            screen_y = i;
            update_screen_char();
        }
        for (i = 0; i < SCREEN_WIDTH; i++)
        {
            screen_x = i;
            screen_y = 0;
            update_screen_char();
            screen_x = i;
            screen_y = SCREEN_HEIGHT - 1;
            update_screen_char();
        }
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
        case KEY_LEFT:
            dt = dt > 1 ? dt - 1 : dt;
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case KEY_RIGHT:
            dt++;
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case '1':
            if (projection != ASIS_XY)
            {
                projection = ASIS_XY;
                reinitialize_path_history(CALL_RUN);
            }
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case '2':
            if (projection != ASIS_XZ)
            {
                projection = ASIS_XZ;
                reinitialize_path_history(CALL_RUN);
            }
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case '3':
            if (projection != ASIS_YZ)
            {
                projection = ASIS_YZ;
                reinitialize_path_history(CALL_RUN);
            }
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case 'R':
            program_state = RESTART;
            break;
        case 'S':
            toggle_stats();
            ignore_button_cooldown = IGNORE_BUTTON_COOLDOWN;
            break;
        case KEY_DEL:
            gal_cls();
            program_state = EXIT_PROGRAM;
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
    gal_cls();

    initialize_globals(CALL_INIT);
    reinitialize_path_history(CALL_INIT);

    welcome_screen();

SIM_ITER:
    // Compute derivatives
    dx = FIXED_MUL(SIGMA, (y - x));
    dy = FIXED_MUL(x, (RO - z)) - y;
    dz = FIXED_MUL(x, y) - FIXED_MUL(BETA, z);

    // Update state
    x += FIXED_MUL(dx, dt);
    y += FIXED_MUL(dy, dt);
    z += FIXED_MUL(dz, dt);

    // Map system state to grid coordinates
    // NOTE: It's incredible that the Lorenz attractor range of values for
    // all three variables fits perfectly within the Galaksija screen bounds.
    // There is no need for even a slight scale adjustment.
    // NOTE: The compiler incorrectly handles the assignment of a 32-bit expression
    // to a char variable, so a helper variable is used to store the value before
    // assigning it to the grid coordinate variables.
    switch (projection)
    {
        case ASIS_XY:
            dx = GRID_WIDTH_HALF + FROM_FIXED(x);
            grid_x = (char)dx;
            dy = GRID_HEIGHT_HALF - FROM_FIXED(y);
            grid_y = (char)dy;
            break;
        case ASIS_XZ:
            dx = GRID_WIDTH_HALF + FROM_FIXED(x);
            grid_x = (char)dx;
            dy = GRID_HEIGHT_HALF - (FROM_FIXED(z) - 25);
            grid_y = (char)dy;
            break;
        case ASIS_YZ:
            dy = GRID_WIDTH_HALF + FROM_FIXED(y);
            grid_x = (char)dy;
            dy = GRID_HEIGHT_HALF - (FROM_FIXED(z) - 25);
            grid_y = (char)dy;
            break;
    }

    // Update the screen and path history if the point is within screen bounds
    // and has moved since the last iteration
    if (grid_x >= 1 && grid_x < GRID_WIDTH - 1 &&
        grid_y >= 1 && grid_y < GRID_HEIGHT - 1 &&
        (grid_x != positions_x[path_index] || grid_y != positions_y[path_index]))
    {
        // If the old and the new positions are not neighbors, fill in the gap.
        temp_grid_x = positions_x[path_index] == -1 ? grid_x - 1 : positions_x[path_index];
        temp_grid_y = positions_y[path_index] == -1 ? grid_y - 1 : positions_y[path_index];
        while (abs(temp_grid_x - grid_x) > 1 || abs(temp_grid_y - grid_y) > 1)
        {
            while (temp_grid_x < 1 || temp_grid_x >= GRID_WIDTH - 1 ||
                temp_grid_y < 1 || temp_grid_y >= GRID_HEIGHT - 1)
            {
                if (temp_grid_x < grid_x) temp_grid_x++;
                else if (temp_grid_x > grid_x) temp_grid_x--;
                if (temp_grid_y < grid_y) temp_grid_y++;
                else if (temp_grid_y > grid_y) temp_grid_y--;
            }

            // Erase oldest point in path history
            oldest_path_index = (path_index + 1) & (PATH_LENGTH - 1);
            if (positions_x[oldest_path_index] != -1 && positions_y[oldest_path_index] != -1)
            {
                visit_count[positions_y[oldest_path_index]][positions_x[oldest_path_index]]--;
                screen_x = positions_x[oldest_path_index] / 2;
                screen_y = positions_y[oldest_path_index] / 3;
                update_screen_char();
            }

            if (temp_grid_x < grid_x) temp_grid_x++;
            else if (temp_grid_x > grid_x) temp_grid_x--;
            if (temp_grid_y < grid_y) temp_grid_y++;
            else if (temp_grid_y > grid_y) temp_grid_y--;

            // Move to the next position in the path history
            path_index = oldest_path_index;

            // Store new position in history
            positions_x[path_index] = temp_grid_x;
            positions_y[path_index] = temp_grid_y;
            visit_count[temp_grid_y][temp_grid_x]++;

            // Update the screen character at new position
            screen_x = temp_grid_x / 2;
            screen_y = temp_grid_y / 3;
            update_screen_char();
        }

        // Erase oldest point in path history
        oldest_path_index = (path_index + 1) & (PATH_LENGTH - 1);
        if (positions_x[oldest_path_index] != -1 && positions_y[oldest_path_index] != -1)
        {
            visit_count[positions_y[oldest_path_index]][positions_x[oldest_path_index]]--;
            screen_x = positions_x[oldest_path_index] / 2;
            screen_y = positions_y[oldest_path_index] / 3;
            update_screen_char();
        }

        // Move to the next position in the path history
        path_index = oldest_path_index;

        // Store new position in history
        positions_x[path_index] = grid_x;
        positions_y[path_index] = grid_y;
        visit_count[grid_y][grid_x]++;

        // Update the screen character at new position
        screen_x = grid_x / 2;
        screen_y = grid_y / 3;
        update_screen_char();
    }

    // Update stats display if enabled
    if (print_stats == STATS_ON)
    {
        gal_gotoxy(3, SCREEN_HEIGHT - 6);
        print_int_3((int)FROM_FIXED(x));

        gal_gotoxy(3, SCREEN_HEIGHT - 5);
        print_int_3((int)FROM_FIXED(y));
        gal_gotoxy(3, SCREEN_HEIGHT - 4);
        print_int_3((int)FROM_FIXED(z));

        gal_gotoxy(4, SCREEN_HEIGHT - 3);
        print_int_3((int)dt);

        gal_gotoxy(7, SCREEN_HEIGHT - 2);
        print_positive_int(iteration);
    }
    iteration++;

    handle_user_input();

    // Restart the simulation, but skip the welcome screen
    if (program_state == RESTART)
    {
        initialize_globals(CALL_RUN);
        reinitialize_path_history(CALL_RUN);
        program_state = SIMULATION;

        // Clear the iteration count to avoid artifacts in stats display
        gal_gotoxy(7, SCREEN_HEIGHT - 2);
        gal_puts("      ");
    }

    if (program_state != EXIT_PROGRAM)
    {
        goto SIM_ITER;
    }

    return 0;
}
