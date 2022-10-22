#include "common.hpp"

void update()
{
	int8_t dx = 0, dy = 0;
	if(btns_down & BTN_UP   ) dy -= 1;
	if(btns_down & BTN_DOWN ) dy += 1;
	if(dy == 0)
	{
		if(btns_down & BTN_LEFT) dx -= 1;
		if(btns_down & BTN_RIGHT) dx += 1;
	}

	pmoving = !(dx == 0 && dy == 0);

	if(pmoving)
	{
		px += dx;
		py += dy;

		if( tile_is_solid(px - 3, py + 1) ||
			tile_is_solid(px + 3, py + 1) ||
			tile_is_solid(px - 3, py + 7) ||
			tile_is_solid(px + 3, py + 7))
		{
			px -= dx;
			py -= dy;
		}

		if(dy < 0)
			pdir = 0;
		else if(dy > 0)
			pdir = 1;
		else if(dx < 0)
			pdir = 2;
		else if(dx > 0)
			pdir = 3;
	}
	update_chunks();

	++nframe;
}
