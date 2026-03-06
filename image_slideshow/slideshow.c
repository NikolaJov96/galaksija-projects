/* Image slideshow for the Galaksija computer.
   Images are baked in at compile time via images.h / images.c,
   generated from arbitrary input images by convert_images.py.

   Controls:
     RIGHT / any key - advance to the next image immediately
     LEFT            - go to the previous image immediately
     DEL             - exit the slideshow and clear the screen
*/

#include <conio.h>

#include "galaksija.h"
#include "images.h"
#include "welcome_screen.h"

/* Number of outer delay loop iterations between image advances.
   Each outer iteration contains 1000 inner iterations plus one getk() call.
   Increase this value for a longer display time per image. */
#define DELAY_COUNT 600

#define KEY_LEFT 45
#define KEY_RIGHT 46
#define KEY_DEL  67

/* Loop counters are global to avoid stack allocation */
int delay_i;
int delay_j;

int current_image;

/* Copy one image from the baked-in array directly to video memory */
void display_image()
{
    int i;
    const unsigned char *img = images[current_image];
    for (i = 0; i < IMAGE_SIZE; i++)
    {
        z80_bpoke(SCREEN_ADDR + i, img[i]);
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
        display_image();
        gal_gotoxy(1, 15);
        gal_puts(image_names[current_image]);

        /* Delay loop: also polls for key presses so the user can exit or
           skip to the next image at any point during the display period */
        for (delay_i = 0; delay_i < DELAY_COUNT; delay_i++)
        {
            for (delay_j = 0; delay_j < 1000; delay_j++);

            key = getk();
            if (key == KEY_DEL)
            {
                gal_cls();
                return 0;
            }
            if (key == KEY_LEFT)
            {
                current_image = current_image > 0 ? current_image - 1 : NUM_IMAGES - 1;
                break;
            }
            if (key != 0)
            {
                current_image++;
                if (current_image >= NUM_IMAGES)
                {
                    current_image = 0;
                }
                break;
            }
        }

        if (key == 0)
        {
            current_image++;
            if (current_image >= NUM_IMAGES)
            {
                current_image = 0;
            }
        }
    }

    return 0;
}
