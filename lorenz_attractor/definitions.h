/* Header file with global defines, enums, and variables */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

// Screen dimensions in the number of characters.
// This is how Galaksija video memory is indexed.

#define SCREEN_WIDTH 32
#define SCREEN_WIDTH_HALF (SCREEN_WIDTH / 2)
#define SCREEN_HEIGHT 16
#define SCREEN_HEIGHT_HALF (SCREEN_HEIGHT / 2)

// Grid dimensions in the number of "pixels".
// Each character represents a 2x3 pixel block.
// Galaksija provides a char for every combination of these 6 pixels,
// so we can work with the higher resolution internally and map it to
// characters only when drawing to the screen.

#define GRID_WIDTH (SCREEN_WIDTH * 2)
#define GRID_WIDTH_HALF (GRID_WIDTH / 2)
#define GRID_HEIGHT (SCREEN_HEIGHT * 3)
#define GRID_HEIGHT_HALF (GRID_HEIGHT / 2)

#define KEY_ENTER 10
#define KEY_LEFT 45
#define KEY_RIGHT 46
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_S 83

#define PATH_LENGTH_SHIFT 8
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

/* Marks the camera view angle of the system */
enum view_axis { ASIS_XY, ASIS_XZ, ASIS_YZ };

/* Marks whether statistics are visible */
enum stats_visibility { STATS_OFF, STATS_ON };

/* At which stage of the program execution is function called */
enum call_mode { CALL_INIT, CALL_RUN };

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

#endif // DEFINITIONS_H
