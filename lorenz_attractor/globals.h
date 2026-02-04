/* Header file with global defines, enums, and variables */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

// Screen dimensions in the number of characters.
// This is how Galaksija video memory is indexed.

#define SCREEN_WIDTH 32
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
#define KEY_DEL 67

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
#define FP_SHIFT 10
#define TO_FIXED(x) ((int64_t)((x) * (1l << FP_SHIFT)))
#define FROM_FIXED(x) ((x) >> FP_SHIFT)
/* Multiplication of two fixed point numbers results in a double
   shifted value, so it needs to be shifted one step back */
#define FIXED_MUL(a, b) (FROM_FIXED(((a) * (b))))

/* Defines states of the program */
enum program_execution_state { TITLE_SCREEN, HELP_SCREEN, SIMULATION, RESTART, EXIT_PROGRAM };

/* Marks the camera view angle of the system */
enum view_axis { ASIS_XY, ASIS_XZ, ASIS_YZ };

/* Marks whether statistics are visible */
enum stats_visibility { STATS_OFF, STATS_ON };

/* At which stage of the program execution is function called */
enum call_mode { CALL_INIT, CALL_RUN };

// Path history tracking variables

/* History of the last PATH_LENGTH X screen coordinates */
extern char positions_x[PATH_LENGTH];
/* History of the last PATH_LENGTH Y screen coordinates */
extern char positions_y[PATH_LENGTH];
/* Simulation can pass multiple times through a same pixel,
   so we keep track of how many times each pixel was visited */
extern unsigned char visit_count[GRID_HEIGHT][GRID_WIDTH];
/* Index of the current position in revolving path history arrays */
extern int path_index;
/* Helper variable containing the index of the oldest position in path history */
extern int oldest_path_index;

// Display utility variables

/* Current program state */
extern enum program_execution_state program_state;
/* Stats visiblity flag */
extern enum stats_visibility print_stats;
/* Cooldown required between registering button presses */
extern unsigned char ignore_button_cooldown;
/* Latest pressed button character */
extern unsigned char char_input;
/* Simulation iteration counter */
extern int iteration;

// Lorenz attractor parameters
// Parameters are hardcoded to values that produce the desired chaotic behavior

/* The first of the Lorenz attractor parameters */
extern int64_t ro;
/* The second of the Lorenz attractor parameters */
extern int64_t sigma;
/* The third of the Lorenz attractor parameters */
extern int64_t beta;
/* Simulation time step */
extern int64_t dt;

// System state variables

/* The X coordinate of the system */
extern int64_t x;
/* The Y coordinate of the system */
extern int64_t y;
/* The Z coordinate of the system */
extern int64_t z;
/* Current camera projection */
extern enum view_axis projection;

// Simulation helper variables

/* Change in X coordinate in the current step */
extern int64_t dx;
/* Change in Y coordinate in the current step */
extern int64_t dy;
/* Change in Z coordinate in the current step */
extern int64_t dz;
/* Grid X coordinate of the system */
extern char grid_x;
/* Grid Y coordinate of the system */
extern char grid_y;
/* Temporary grid X coordinate of the system */
extern char temp_grid_x;
/* Temporary grid Y coordinate of the system */
extern char temp_grid_y;
/* Screen X coordinate of the system */
extern char screen_x;
/* Screen Y coordinate of the system */
extern char screen_y;

/* Initializes all global variables to their default states.
   If the values were initialized in compile time, they would only be set when
   the program is first loaded using the OLD command. Successive restarts of
   the program would run with memory in the state left by the previous run. */
void initialize_globals(enum call_mode call_mode);

#endif // DEFINITIONS_H
