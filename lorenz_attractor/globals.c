/* Definitions of global variables declared in the corresponding header file */

#include "globals.h"

#include <stdlib.h>

#include "galaksija.h"

int32_t x, y, z;
int32_t dt, dx, dy, dz;
char grid_x, grid_y, temp_grid_x, temp_grid_y, screen_x, screen_y;

enum program_execution_state program_state;
enum view_axis projection;
enum stats_visibility print_stats;
unsigned char previous_button;
int iteration;

int path_index;
int oldest_path_index;
char positions_x[PATH_LENGTH];
char positions_y[PATH_LENGTH];
unsigned char visit_count[GRID_HEIGHT][GRID_WIDTH];

void initialize_globals(enum call_mode call_mode)
{
    // Skip the following initializations if called from a restart
    if (call_mode == CALL_INIT)
    {
        srand(z80_wpeek(RND_ADDR));

        dt = TO_FIXED(0.01);

        program_state = TITLE_SCREEN;
        projection = ASIS_XZ;
        print_stats = STATS_OFF;
    }

    previous_button = 0;
    iteration = 1;
    path_index = 0;
    oldest_path_index = 0;

    x = TO_FIXED(rand() / (RAND_MAX / 40) - 20);
    y = TO_FIXED(rand() / (RAND_MAX / 40) - 20);
    z = TO_FIXED(rand() / (RAND_MAX / 40) + 5);
}
