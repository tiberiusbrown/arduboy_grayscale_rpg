import pytmx
import itertools

CHUNKS_W = 32
CHUNKS_H = 32
CHUNK_SCRIPT_SIZE = 32

TILES_W = CHUNKS_W * 8
TILES_H = CHUNKS_H * 4

tm = pytmx.TiledMap('world.tmx')

def get_tile_id(x, y):
    im = tm.get_tile_image(x, y, 0)
    if im is None:
        return 30
    return im[1][1] + im[1][0] // 16

chunks = [[0] * (4 * 8 + CHUNK_SCRIPT_SIZE) for i in range(32 * 32)]

for y in range(TILES_H):
    cy = y // 4
    ty = y % 4
    for x in range(TILES_W):
        cx = x // 8
        tx = x % 8
        c = cy * CHUNKS_W + cx
        t = ty * 8 + tx
        id = get_tile_id(x, y)
        chunks[c][t] = id
        
bytes = bytearray(itertools.chain.from_iterable(chunks))

# write header file for emulated FX data
with open('../src/generated/fxdata_emulated.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('constexpr uint8_t FXDATA[] =\n{\n')
    for i in range(len(bytes)):
        f.write('%3d,%s' % (bytes[i], '' if i % 16 < 15 else '\n'))
    f.write('};\n')
    
# write fxdata.bin
with open('../arduboy_build/rpgdata.bin', 'wb') as f:
    f.write(bytes)
