/* Definitions of global variables declared in the corresponding header file */

#include "globals.h"

// Path history tracking variables

/* History of the last PATH_LENGTH X screen coordinates */
char positions_x[PATH_LENGTH];
/* History of the last PATH_LENGTH Y screen coordinates */
char positions_y[PATH_LENGTH];
/* Simulation can pass multiple times through a same pixel,
   so we keep track of how many times each pixel was visited */
unsigned char visit_count[GRID_HEIGHT][GRID_WIDTH];
/* Index of the current position in revolving path history arrays */
int path_index = 0;
/* Helper variable containing the index of the oldest position in path history */
int oldest_path_index = 0;

// Display utility variables

/* Stats visiblity flag */
enum stats_visibility print_stats = STATS_OFF;
/* Cooldown required between registering button presses */
unsigned char ignore_button_cooldown = 0;
/* Latest pressed button character */
unsigned char char_input;
/* Simulation iteration counter */
uint32_t iteration = 1;

// Lorenz attractor parameters
// Parameters are hardcoded to values that produce the desired chaotic behavior

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
int32_t x = TO_FIXED(10.0);
/* The Y coordinate of the system */
int32_t y = TO_FIXED(10.0);
/* The Z coordinate of the system */
int32_t z = TO_FIXED(10.0);
/* Current camera projection */
enum view_axis projection = ASIS_XZ;

// Simulation helper variables

/* Change in X coordinate in the current step */
int32_t dx;
/* Change in Y coordinate in the current step */
int32_t dy;
/* Change in Z coordinate in the current step */
int32_t dz;
/* Grid X coordinate of the system */
char grid_x;
/* Grid Y coordinate of the system */
char grid_y;
/* Temporary grid X coordinate of the system */
char temp_grid_x;
/* Temporary grid Y coordinate of the system */
char temp_grid_y;
/* Screen X coordinate of the system */
char screen_x;
/* Screen Y coordinate of the system */
char screen_y;
