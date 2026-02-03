# Lorenz System

Simulation and visualization of a Lorenz system using fixed-point arithmetic.

Learn about chaotic systems and Lorenz attractors on this [wiki](https://en.wikipedia.org/wiki/Lorenz_system).

## Build

Build using the following Z88DK command:

``` bash
zcc +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic -o lorenz lorenz.c galaksija.c globals.c welcome_screen.c
```

## TODO

- Use different chars for path history fading
- Separate screen for command help
- Examine the effect on performance of using macros instead of functions
