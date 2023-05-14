import itertools
import script_assembler
from pathlib import Path
import sys
import re
from PIL import Image
import os

import pytmx
tm = pytmx.TiledMap('world.tmx')

CHUNKS_W = 32
CHUNKS_H = 64
CHUNK_SCRIPT_SIZE = 80
CHUNK_ENEMY_PATH_SIZE = 8

TILES_W = CHUNKS_W * 8
TILES_H = CHUNKS_H * 4

CHUNK_BYTES = 32 + CHUNK_SCRIPT_SIZE
#CHUNK_BYTES = 32 + CHUNK_SCRIPT_SIZE + 2 + 1 + 1 + CHUNK_ENEMY_PATH_SIZE
CHUNK_SCRIPT_OFFSET = 32
#CHUNK_ENEMY_OFFSET = CHUNK_SCRIPT_OFFSET + CHUNK_SCRIPT_SIZE

def get_tile_id(x, y):
    im = tm.get_tile_image(x, y, 0)
    if im is None: return 0
    return (im[1][0] // 16 + im[1][1]) & 0xff

chunks = [[0] * CHUNK_BYTES for i in range(CHUNKS_W * CHUNKS_H)]

# map image
renew_map_image = True
if os.path.exists('world.png'):
    imgtime = os.path.getmtime('world.tmx')
    outtime = os.path.getmtime('world.png')
    tilestime = os.path.getmtime('tiles_small.png')
    if outtime > imgtime and outtime > tilestime:
        renew_map_image = False

if renew_map_image:
    tiles_small = Image.open('tiles_small.png').convert('RGB')
    map_image = Image.new(mode="RGB", size=(TILES_W*2, TILES_H*2//2))

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
        
        if not renew_map_image:
            continue
        ox = (id %  16) * 2
        oy = (id // 16) * 2
        if oy < TILES_H*2//2:
            map_image.paste(
                tiles_small.copy().crop(box=(ox, oy, ox + 2, oy + 2)),
                box=(x * 2, y * 2))

if renew_map_image:
    map_image.save('world.png')
        
bytes = bytearray(itertools.chain.from_iterable(chunks))

bs = [[] for x in range(CHUNKS_W * CHUNKS_H)]

def pixel_to_chunk(x, y):
    return int(y) // 64 * CHUNKS_W + int(x) // 128
    
def pixel_to_tile(x, y):
    return ((int(y) % 64) // 16) * 8 + ((int(x) % 128) // 16)

epaths = [{} for x in range(CHUNKS_W * CHUNKS_H)]

script_assembler.init()

def convert_path(obj):
    chunk = pixel_to_chunk(obj.x, obj.y)
    if hasattr(obj, 'points'):
        tiles = [pixel_to_tile(pt.x, pt.y) for pt in obj.points]
        chunks = [pixel_to_chunk(pt.x, pt.y) for pt in obj.points]
        for c in chunks:
            if c != chunk:
                print('Path between two chunks at %d,%d' % (obj.x, obj.y))
                sys.exit(1)
        i = 1
        openpath = 0 if obj.closed else 1
        if not openpath and not(
                ((tiles[0] & 0x07) == (tiles[-1] & 0x07)) or
                ((tiles[0] & 0x18) == (tiles[-1] & 0x18))):
            print('Non orthogonal path at %d,%d' % chunk)
        while i < len(tiles):
            if not(
                    ((tiles[i] & 0x07) == (tiles[i-1] & 0x07)) or
                    ((tiles[i] & 0x18) == (tiles[i-1] & 0x18))):
                print('Non orthogonal path at %d,%d' % (obj.x, obj.y))
            if tiles[i] == tiles[i-1] & 0x1f:
                tiles[i-1] += 0x20
                if tiles[i-1] & 0xe0 == 0:
                    print('Too many path delays at %d,%d' % (obj.x, obj.y))
                    sys.exit(1)
                tiles.pop(i)
            else:
                i += 1
        if tiles[0] == tiles[-1] & 0x1f:
            if (tiles[0] >> 5) + (tiles[-1] >> 5) > 7:
                print('Too many path delays at %d,%d' % (obj.x, obj.y))
                sys.exit(1)
            tiles[0] += (tiles[-1] & 0xe0)
            tiles.pop(-1)
        if len(tiles) > CHUNK_ENEMY_PATH_SIZE:
            print('Path too long at %d,%d' % (obj.x, obj.y))
            sys.exit(1)
    else:
        #TODO: point shapes
        print('unknown path object, chunk %d' % chunk)
        sys.exit(1)
    return tiles

# convert all sprite paths
for obj in tm.layers[2]:
    chunk = pixel_to_chunk(obj.x, obj.y)
    tiles = convert_path(obj)
    openpath = 0 if obj.closed else 1
    if obj.name is not None and obj.name[0] == '!':
        if not hasattr(obj, 'class'):
            print('flagged sprite path missing class attribute: (%d, %d)' % (obj.x, obj.y))
            sys.exit(1)
        f = script_assembler.flag(obj.name)
        bs[chunk] += [script_assembler.CMD.EPF._value_]
        bs[chunk] += [(f >> 0) % 256]
        bs[chunk] += [(f >> 8) % 256]
        bs[chunk] += [script_assembler.sprite(getattr(obj, 'class'))]
        bs[chunk] += [len(tiles), openpath] + tiles
        continue
    elif hasattr(obj, 'class'):
        bs[chunk] += [script_assembler.CMD.EP._value_]
        bs[chunk] += [script_assembler.sprite(getattr(obj, 'class'))]
        bs[chunk] += [len(tiles), openpath] + tiles
    epaths[chunk][obj.name] = [len(tiles), openpath] + tiles

locations = {}
# record all locations
for obj in tm.layers[3]:
    x = int(obj.x)
    y = int(obj.y)
    tx = x // 16
    ty = y // 16
    locations[obj.name] = '%d %d' % (tx, ty)
    
with open('../src/generated/locations.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    for k in locations:
        t = locations[k].split()
        f.write('inline void LOC_%-32s() { px = %5d; py = %5d; }\n' %
            (k[1:], int(t[0]) * 16, int(t[1]) * 16))
    
def locreplace(match):
    return locations[match.group(0)]
    
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
            t = t.replace('$bnst ', 'bnst $T ')
            t = t.replace('$bnwt ', 'bnwt $T ')
            t = t.replace('$st ', 'st $T ')
            t = t.replace('$stf ', 'stf $T ')
            t = t.replace('$CHEST ', 'CHEST $T ')
            t = t.replace('$POT ', 'POT $T ')
            t = t.replace('$T', str(tile))
            t = re.sub('|'.join(r'@\b%s\b' % re.escape(loc[1:]) for loc in locations), 
                locreplace, t)
            s = s + t + '\n'
    b = script_assembler.assemble(s, epaths[chunk], (x, y))
    bs[chunk] += b
    if len(bs[chunk]) > CHUNK_SCRIPT_SIZE:
        print('Script too large at chunk: %d,%d' %(obj.x, obj.y))
        print('Size: %d' % len(bs[chunk]))
        sys.exit(1)

for chunk in range(CHUNKS_W * CHUNKS_H):
    index = CHUNK_BYTES * chunk + CHUNK_SCRIPT_OFFSET
    b = bs[chunk]
    #if len(b) > 0: print(b)
    for i in range(len(b)):
        bytes[index + i] = b[i]

def atoi(text):
    return int(text) if text.isdigit() else text
def natural_keys(text):
    return [ atoi(c) for c in re.split(r'(\d+)', text) ]

# write story_flags.hpp
with open('../src/generated/story_flags.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    n = len(script_assembler.flags)
    n = 1 if n == 0 else (n + 7) / 8
    f.write('constexpr int STORY_FLAG_BYTES = %d;\n\n' % n)
    for k in sorted(script_assembler.flags, key=natural_keys):
        if k[0] != '!': continue
        f.write('constexpr uint16_t SFLAG_%s = %d;\n' %
            (k[1:], script_assembler.flags[k]))

# write mapdata.bin
with open('../arduboy_build/mapdata.bin', 'wb') as f:
    f.write(bytes)
    
# write stringdata.bin
with open('../arduboy_build/stringdata.bin', 'wb') as f:
    f.write(bytearray(script_assembler.strings))
    