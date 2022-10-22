#include "common.hpp"

#include "tile_solid.hpp"

static void load_chunk(uint8_t index, uint8_t cx, uint8_t cy)
{
	uint16_t ci = cy * MAP_CHUNK_W + cx;
	uint24_t addr = uint24_t(ci) * sizeof(map_chunk_t);
	platform_fx_read_data_bytes(
		addr, &active_chunks[index].chunk, sizeof(map_chunk_t));
}

void update_chunks()
{
	uint8_t cx = uint8_t(uint16_t(px - 64) >> 7);
	uint8_t cy = uint8_t(uint16_t(py - 32) >> 6);

	// TODO: optimize shifting map
	//if(loaded_cx == cx && loaded_cy == cy)
	//	return;

	loaded_cx = cx;
	loaded_cy = cy;

	load_chunk(0, cx + 0, cy + 0);
	load_chunk(1, cx + 1, cy + 0);
	load_chunk(2, cx + 0, cy + 1);
	load_chunk(3, cx + 1, cy + 1);
}

bool tile_is_solid(uint16_t tx, uint16_t ty)
{
	uint8_t cx = uint8_t(tx >> 7);
	uint8_t cy = uint8_t(ty >> 6);
	cx -= loaded_cx;
	cy -= loaded_cy;
	if(cx > 1 || cy > 1) return true;
	uint8_t x = uint8_t(tx & 0x7f) >> 4;
	uint8_t y = uint8_t(ty & 0x3f) >> 4;
	uint8_t ci = cy * 2 + cx;
	uint8_t t = active_chunks[ci].chunk.tiles[y][x];
	t = pgm_read_byte(&TILE_SOLID[t]);
	x = 1;
	if(ty & 0x08) x <<= 2;
	if(tx & 0x08) x <<= 1;
	return (t & x) != 0;
}
