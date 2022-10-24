import itertools
import script_assembler
from pathlib import Path
import sys

import pytmx
tm = pytmx.TiledMap('world.tmx')

CHUNKS_W = 32
CHUNKS_H = 32
CHUNK_SCRIPT_SIZE = 32

TILES_W = CHUNKS_W * 8
TILES_H = CHUNKS_H * 4

def get_tile_id(x, y):
    im = tm.get_tile_image(x, y, 0)
    if im is None: return 30
    return im[1][0] // 16 + im[1][1]
    #t = tm.layers[0].data[y][x]
    #return 30 if t == 0 else t - 1

chunks = [[0] * (32 + CHUNK_SCRIPT_SIZE) for i in range(CHUNKS_W * CHUNKS_H)]

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

bs = [[] for x in range(CHUNKS_W * CHUNKS_H)]

for obj in tm.layers[1]:
    x = int(obj.x)
    y = int(obj.y)
    chunk = y // 64 * CHUNKS_W + x // 128
    tile = ((y % 64) // 16) * 8 + ((x % 128) // 16)
    s = ''
    for p,v in obj.properties.items():
        if p.startswith('scr'):
            t = v
            t = t.replace('$tmsg ', 'tmsg $T ')
            t = t.replace('$tdlg ', 'tdlg $T ')
            t = t.replace('$ttp ', 'ttp $T ')
            t = t.replace('$wtp ', 'wtp $T ')
            t = t.replace('$T', str(tile))
            s = s + t + '\n'
    b = script_assembler.assemble(s)
    bs[chunk] += b
    if len(b) > CHUNK_SCRIPT_SIZE:
        print('Script too large:\n\n%s\n\n' % s)
        print(obj.coordinates)
        sys.exit(1)
        
for chunk in range(CHUNKS_W * CHUNKS_H):
    index = (32 + CHUNK_SCRIPT_SIZE) * chunk + 32
    b = bs[chunk]
    for i in range(len(b)):
        bytes[index + i] = b[i]

# write mapdata.bin
with open('../arduboy_build/mapdata.bin', 'wb') as f:
    f.write(bytes)
    
# write stringdata.bin
with open('../arduboy_build/stringdata.bin', 'wb') as f:
    f.write(bytearray(script_assembler.strings))
    