![galaksija](https://github.com/user-attachments/assets/70c6325e-af96-4981-b123-3ce4ea6c6796)

# Projects for the Galaksija computer

This repo contains projects for the Galaksija computer.

Galaksija is an 8-bit DIY computer from the 1980s, based on the Z80 CPU.

In 2025, the creators of the original computer released an updated specification that uses currently available electronic components, expands RAM and ROM, and remains completely backwards compatible with original programs.

Resources:
- [Wikipedia](https://en.wikipedia.org/wiki/Galaksija_(computer))
- [The new specification](https://racunari.com/galaksija/)
- Ivana Iličev [GitHub](https://github.com/ivang78) with some cool programs in C

## Building the projects

Projects written in C can be built using the Z88DK development kit, which is a C compiler for 8-bit hardware, with explicit support for Galaksija.

It can be installed from its [GitHub](https://github.com/z88dk/z88dk) page.

Running the build will generate three outputs:
- Binary file with no extension
- `.gtp` tape file that can be loaded into an emulator or onto Galaksija 2025
- `.wav` audio file that can be saved to cassette tape for loading onto the original Galaksija

## Running the projects

### Original hardware

The Galaksija 2025 has a TTL serial port and can be connected to a PC using a TTL-USB converter.

Built `.gtp` files can be loaded into RAM using scripts provided [here](https://racunari.com/GalPCUtils.zip).

### Emulators

The following online emulators are completely compatible with the original hardware:

- [Emulator 1](https://galaksija.net/)
- [Emulator 2](https://galaksija.epizy.com/Galaksija/jsgalmin_lite.html)

## Heritage programming peculiarities

- All arrays are defined globally because stack memory is too limited.
- For consistency, all other variables are defined globally, too, except loop iterators.
- Defines and code duplication are not sins, because every cycle counts, and function calls are not free.
- Int 32 and int 64 arithmetic works, but assigning an expression of one type to a variable of another type may cause inconsistent crashes.
  ``` C
  int32_t x, y;

  // Incorrect
  char r = x + y;

  // Correct
  int32_t temp = x + y;
  char r = (char)temp;
  ```
