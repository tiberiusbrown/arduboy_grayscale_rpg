import itertools
import script_assembler
from pathlib import Path
import sys

import pytmx
tm = pytmx.TiledMap('world.tmx')

CHUNKS_W = 32
CHUNKS_H = 32
CHUNK_SCRIPT_SIZE = 32
CHUNK_ENEMY_PATH_SIZE = 8

TILES_W = CHUNKS_W * 8
TILES_H = CHUNKS_H * 4

CHUNK_BYTES = 32 + CHUNK_SCRIPT_SIZE + 2 + 1 + 1 + CHUNK_ENEMY_PATH_SIZE
CHUNK_SCRIPT_OFFSET = 32
CHUNK_ENEMY_OFFSET = CHUNK_SCRIPT_OFFSET + CHUNK_SCRIPT_SIZE

def get_tile_id(x, y):
    im = tm.get_tile_image(x, y, 0)
    if im is None: return 30
    return im[1][0] // 16 + im[1][1]

chunks = [[0] * CHUNK_BYTES for i in range(CHUNKS_W * CHUNKS_H)]

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

def pixel_to_chunk(x, y):
    return int(y) // 64 * CHUNKS_W + int(x) // 128
    
def pixel_to_tile(x, y):
    return ((int(y) % 64) // 16) * 8 + ((int(x) % 128) // 16)

# assemble all script objects
for obj in tm.layers[1]:
    x = int(obj.x)
    y = int(obj.y)
    chunk = pixel_to_chunk(x, y)
    tile = pixel_to_tile(x, y)
    s = ''
    for p,v in obj.properties.items():
        if p.startswith('scr'):
            t = v
            t = t.replace('$tmsg ', 'tmsg $T ')
            t = t.replace('$tdlg ', 'tdlg $T ')
            t = t.replace('$ttp ', 'ttp $T ')
            t = t.replace('$wtp ', 'wtp $T ')
            t = t.replace('$brnt ', 'brnt $T ')
            t = t.replace('$brnw ', 'brnw $T ')
            t = t.replace('$st ', 'st $T ')
            t = t.replace('$T', str(tile))
            s = s + t + '\n'
    b = script_assembler.assemble(s)
    bs[chunk] += b
    if len(bs[chunk]) > CHUNK_SCRIPT_SIZE:
        print('Script too large at chunk: %d,%d' %(obj.x, obj.y))
        sys.exit(1)

for chunk in range(CHUNKS_W * CHUNKS_H):
    index = CHUNK_BYTES * chunk + CHUNK_SCRIPT_OFFSET
    b = bs[chunk]
    #if len(b) > 0: print(b)
    for i in range(len(b)):
        bytes[index + i] = b[i]

# convert all enemy paths
for obj in tm.layers[2]:
    chunk = pixel_to_chunk(obj.x, obj.y)
    if hasattr(obj, 'points'):
        tiles = [pixel_to_tile(pt.x, pt.y) for pt in obj.points]
        chunks = [pixel_to_chunk(pt.x, pt.y) for pt in obj.points]
        for c in chunks:
            if c != chunk:
                print('Path between two chunks: chunk %d' % chunk)
                sys.exit(1)
        i = 1
        if not(
                ((tiles[0] & 0x07) == (tiles[-1] & 0x07)) or
                ((tiles[0] & 0x18) == (tiles[-1] & 0x18))):
            print('Non orthogonal path: chunk %d' % chunk)
        while i < len(tiles):
            if not(
                    ((tiles[i] & 0x07) == (tiles[i-1] & 0x07)) or
                    ((tiles[i] & 0x18) == (tiles[i-1] & 0x18))):
                print('Non orthogonal path: chunk %d' % chunk)
            if tiles[i] == tiles[i-1] & 0x1f:
                tiles[i-1] += 0x20
                if tiles[i-1] & 0xe0 == 0:
                    print('Too many path delays: chunk %d' % chunk)
                    sys.exit(1)
                tiles.pop(i)
            else:
                i += 1
        if len(tiles) > CHUNK_ENEMY_PATH_SIZE:
            print('Path too long in chunk %d' % chunk)
            sys.exit(1)
    else:
        #TODO: point shapes
        print('unknown path object, chunk %d' % chunk)
        continue
    index = CHUNK_BYTES * chunk + CHUNK_ENEMY_OFFSET
    f = script_assembler.flag(obj.name)
    if not hasattr(obj, 'class'):
        print('enemy has no class, chunk %d' % chunk)
        sys.exit(1)
    tiles = [
        (f >> 0) % 256,
        (f >> 8) % 256,
        int(getattr(obj, 'class')),
        len(tiles)] + tiles
    for i in range(len(tiles)):
        bytes[index + i] = tiles[i]

# write story_flags.hpp
with open('../src/generated/story_flags.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    n = len(script_assembler.flags)
    n = 1 if n == 0 else (n + 7) / 8
    f.write('constexpr int STORY_FLAG_BYTES = %d;\n' % n)

# write mapdata.bin
with open('../arduboy_build/mapdata.bin', 'wb') as f:
    f.write(bytes)
    
# write stringdata.bin
with open('../arduboy_build/stringdata.bin', 'wb') as f:
    f.write(bytearray(script_assembler.strings))
    