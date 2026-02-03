/* Definitions of global variables declared in the corresponding header file */

#include "globals.h"

char positions_x[PATH_LENGTH];
char positions_y[PATH_LENGTH];
unsigned char visit_count[GRID_HEIGHT][GRID_WIDTH];
int path_index;
int oldest_path_index;

enum program_execution_state program_state;
enum stats_visibility print_stats;
unsigned char ignore_button_cooldown;
unsigned char char_input;
uint32_t iteration;

int32_t ro, sigma, beta, dt;
int32_t x, y, z;
enum view_axis projection;
int32_t dx, dy, dz;
char grid_x, grid_y, temp_grid_x, temp_grid_y, screen_x, screen_y;

void initialize_globals()
{
    path_index = 0;
    oldest_path_index = 0;

    program_state = TITLE_SCREEN;
    print_stats = STATS_OFF;
    ignore_button_cooldown = 0;
    iteration = 1;

    ro = TO_FIXED(28.0);
    sigma = TO_FIXED(10.0);
    beta = TO_FIXED(8.0 / 3.0);
    dt = TO_FIXED(0.01);

    x = TO_FIXED(10.0);
    y = TO_FIXED(10.0);
    z = TO_FIXED(10.0);

    projection = ASIS_XZ;
}
