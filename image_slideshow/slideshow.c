/* Image slideshow for the Galaksija computer.
   Images are baked in at compile time via images.h / images.c,
   generated from arbitrary input images by convert_images.py.

   Each image is stored at MULTIPLIER × the screen resolution. The viewport
   starts at the top-left corner and slowly pans to the bottom-right corner,
   then the next image is shown.

   Controls:
     RIGHT / any key - advance to the next image immediately
     LEFT            - go to the previous image immediately
     DEL             - exit the slideshow and clear the screen
*/

#include <conio.h>

#include "galaksija.h"
#include "images.h"
#include "welcome_screen.h"

/* Pan effect constants.
   PAN_STEPS : number of viewport positions shown per image (pan resolution).
   PAN_DELAY : outer delay loop iterations between each pan step.
               Each outer iteration runs 1000 inner iterations plus a getk() call.
               Increase for a slower pan. */
#define MAX_PAN_X  ((MULTIPLIER - 1) * SCREEN_WIDTH)
#define MAX_PAN_Y  ((MULTIPLIER - 1) * SCREEN_HEIGHT)
#define PAN_STEPS  64
#define PAN_DELAY  15

#define KEY_LEFT 45
#define KEY_RIGHT 46
#define KEY_DEL  67

/* Loop counters and pan state are global to avoid stack allocation */
int delay_i;
int delay_j;
int pan_step;
int pan_x, pan_y;

int current_image;

/* Copy a SCREEN_WIDTH x SCREEN_HEIGHT window from the current image,
   offset by (pan_x, pan_y) chars, directly to video memory. */
void display_window()
{
    int cy, cx;
    const unsigned char *img = images[current_image];
    for (cy = 0; cy < SCREEN_HEIGHT; cy++)
    {
        for (cx = 0; cx < SCREEN_WIDTH; cx++)
        {
            z80_bpoke(SCREEN_ADDR + cy * SCREEN_WIDTH + cx,
                      img[(cy + pan_y) * IMAGE_COLS + (cx + pan_x)]);
        }
    }
}

int main()
{
    unsigned char key;

    gal_cls();
    welcome_screen();
    gal_cls();

    current_image = 0;

    while (1)
    {
        key = 0;

        for (pan_step = 0; pan_step <= PAN_STEPS && key == 0; pan_step++)
        {
            pan_x = pan_step * MAX_PAN_X / PAN_STEPS;
            pan_y = pan_step * MAX_PAN_Y / PAN_STEPS;

            display_window();
            gal_gotoxy(1, 15);
            gal_puts(image_names[current_image]);

            for (delay_i = 0; delay_i < PAN_DELAY && key == 0; delay_i++)
            {
                for (delay_j = 0; delay_j < 1000; delay_j++);
                key = getk();
            }
        }

        if (key == KEY_DEL)
        {
            gal_cls();
            return 0;
        }
        if (key == KEY_LEFT)
        {
            current_image = current_image > 0 ? current_image - 1 : NUM_IMAGES - 1;
        }
        else
        {
            current_image++;
            if (current_image >= NUM_IMAGES)
                current_image = 0;
        }
    }

    return 0;
}
