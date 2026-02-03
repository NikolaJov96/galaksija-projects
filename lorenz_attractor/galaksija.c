/* Implementations of specific GALAKSIJA low-level functions */

#include "galaksija.h"

unsigned char _scr_x, _scr_y;

void gal_cls()
{
	int z;
	for (z = 0; z <512; z++)
	{
		z80_bpoke(SCREEN_ADDR + z, 32);
	}
	_scr_x = 0;
	_scr_y = 0;
}

void gal_gotoxy(char x, char y)
{
	_scr_x = x;
	_scr_y = y;
}

void gal_putc(char ch)
{
	z80_bpoke(SCREEN_ADDR + (_scr_y << 5) + _scr_x, ch);
	_scr_x++;
	if (_scr_x > 32)
	{
		_scr_x = 0;
		_scr_y++;
	}
}

int gal_puts(char *str)
{
	char ch;
	int len = 0;
	while ((ch = *str) != 0x0)
	{
		z80_bpoke(SCREEN_ADDR + (_scr_y << 5) + _scr_x, ch);
		str++;
		len++;
		_scr_x++;
		if (_scr_x > 32)
		{
			_scr_x = 0;
			_scr_y++;
		}
	}
	return len;
}
