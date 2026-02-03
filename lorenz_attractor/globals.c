/* Definitions of global variables declared in the corresponding header file */

#include "globals.h"

// Path history tracking variables

char positions_x[PATH_LENGTH];
char positions_y[PATH_LENGTH];
unsigned char visit_count[GRID_HEIGHT][GRID_WIDTH];
int path_index = 0;
int oldest_path_index = 0;

// Display utility variables

enum program_execution_state program_state = TITLE_SCREEN;
enum stats_visibility print_stats = STATS_OFF;
unsigned char ignore_button_cooldown = 0;
unsigned char char_input;
uint32_t iteration = 1;

// Lorenz attractor parameters
// Parameters are hardcoded to values that produce the desired chaotic behavior

int32_t ro = TO_FIXED(28.0);
int32_t sigma = TO_FIXED(10.0);
int32_t beta = TO_FIXED(8.0 / 3.0);
int32_t dt = TO_FIXED(0.01);

// System state variables

int32_t x = TO_FIXED(10.0);
int32_t y = TO_FIXED(10.0);
int32_t z = TO_FIXED(10.0);
enum view_axis projection = ASIS_XZ;

// Simulation helper variables

int32_t dx;
int32_t dy;
int32_t dz;
char grid_x;
char grid_y;
char temp_grid_x;
char temp_grid_y;
char screen_x;
char screen_y;
