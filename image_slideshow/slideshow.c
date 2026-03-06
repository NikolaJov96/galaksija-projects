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
   MAX_PAN_X/Y  : from images.h — pan travel in chars (IMAGE_COLS/ROWS - SCREEN_WIDTH/HEIGHT).
   PAN_STEPS    : number of pan positions per image. Both axes advance proportionally each
                  step so the viewport always moves along the same angle towards the
                  bottom-right corner: pan_x/pan_y == MAX_PAN_X/MAX_PAN_Y at every step.
   PAN_DELAY    : outer delay loop iterations between each pan step.
                  Each outer iteration runs 1000 inner iterations plus a getk() call.
                  Increase for a slower pan.
   STATIC_DELAY : outer delay iterations when there is no pan (multiplier = 1.0). */
#define PAN_STEPS    8
#define PAN_DELAY    15
#define STATIC_DELAY ((PAN_STEPS + 1) * PAN_DELAY)
#define HOLD_DELAY   30   /* extra outer iterations to hold the last frame before advancing */

#define KEY_LEFT 45
#define KEY_RIGHT 46
#define KEY_DEL  67

/* Loop counters and pan state are global to avoid stack allocation */
int delay_i;
int delay_j;
int pan_step;
int pan_x, pan_y;

int current_image;

/* Write the current image name on the left of the last row.
   The right portion of that row is filled by display_window().
   Called once per image, not on every pan step. */
void display_name()
{
    gal_gotoxy(0, 15);
    gal_puts(image_names[current_image]);
}

/* Copy a SCREEN_WIDTH x SCREEN_HEIGHT window from the current image,
   offset by (pan_x, pan_y) chars, directly to video memory.
   On the last row the name occupies the left columns, so image content
   is written only from column image_name_lengths[current_image] onward. */
void display_window()
{
    int cy, cx, cx_start;
    unsigned char ch;
    const unsigned char *img = images[current_image];
    for (cy = 0; cy < SCREEN_HEIGHT; cy++)
    {
        cx_start = (cy == SCREEN_HEIGHT - 1) ? image_name_lengths[current_image] : 0;
        for (cx = cx_start; cx < SCREEN_WIDTH; cx++)
        {
            ch = img[(cy + pan_y) * IMAGE_COLS + (cx + pan_x)];
            if (cy == SCREEN_HEIGHT - 2 && cx < image_name_lengths[current_image])
                ch &= ~(16 | 32);  /* clear bottom two pixels in chars above the name */
            if (cy == SCREEN_HEIGHT - 1 && cx == image_name_lengths[current_image])
                ch &= ~(1 | 4 | 16);  /* clear left three pixels of char right of name */
            if (cy == SCREEN_HEIGHT - 2 && cx == image_name_lengths[current_image])
                ch &= ~16;  /* clear bottom-left pixel of char above-right of name */
            z80_bpoke(SCREEN_ADDR + cy * SCREEN_WIDTH + cx, ch);
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

        if (MAX_PAN_X == 0)
        {
            /* No pan: display once and hold for the full STATIC_DELAY */
            pan_x = 0;
            pan_y = 0;
            display_window();
            display_name();

            for (delay_i = 0; delay_i < STATIC_DELAY && key == 0; delay_i++)
            {
                for (delay_j = 0; delay_j < 1000; delay_j++);
                key = getk();
            }
        }
        else
        {
            /* Pan from top-left to bottom-right; name is written once before panning */
            pan_x = 0;
            pan_y = 0;
            display_window();
            display_name();

            for (pan_step = 1; pan_step <= PAN_STEPS && key == 0; pan_step++)
            {
                for (delay_i = 0; delay_i < PAN_DELAY && key == 0; delay_i++)
                {
                    for (delay_j = 0; delay_j < 1000; delay_j++);
                    key = getk();
                }

                pan_x = pan_step * MAX_PAN_X / PAN_STEPS;
                pan_y = pan_step * MAX_PAN_Y / PAN_STEPS;
                display_window();
            }

            /* Hold on the final frame */
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

        /* Hold the last frame a bit longer before advancing */
        for (delay_i = 0; delay_i < HOLD_DELAY; delay_i++)
            for (delay_j = 0; delay_j < 1000; delay_j++);
    }

    return 0;
}
