/* Definitions of global variables declared in the corresponding header file */

#include "globals.h"

#include <stdlib.h>

#include "galaksija.h"

char positions_x[PATH_LENGTH];
char positions_y[PATH_LENGTH];
unsigned char visit_count[GRID_HEIGHT][GRID_WIDTH];
int path_index;
int oldest_path_index;

enum program_execution_state program_state;
enum stats_visibility print_stats;
unsigned char ignore_button_cooldown;
unsigned char char_input;
int iteration;

int64_t ro, sigma, beta, dt;
int64_t x, y, z;
enum view_axis projection;
int64_t dx, dy, dz;
char grid_x, grid_y, temp_grid_x, temp_grid_y, screen_x, screen_y;

void initialize_globals(enum call_mode call_mode)
{
    // Skip the following initializations if called from a restart
    if (call_mode == CALL_INIT)
    {
        srand(z80_wpeek(RND_ADDR));

        ro = TO_FIXED(28.0);
        sigma = TO_FIXED(10.0);
        beta = TO_FIXED(8.0 / 3.0);
        dt = TO_FIXED(0.01);

        program_state = TITLE_SCREEN;
        print_stats = STATS_OFF;
        projection = ASIS_XZ;

        ignore_button_cooldown = 0;
    }

    path_index = 0;
    oldest_path_index = 0;
    iteration = 1;

    x = TO_FIXED(rand() / (RAND_MAX / 40) - 20);
    y = TO_FIXED(rand() / (RAND_MAX / 40) - 20);
    z = TO_FIXED(rand() / (RAND_MAX / 40) + 5);
}
