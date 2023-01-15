# tile collision flags

#     0: ..    1: X.    2: .X    3: XX
#        ..       ..       ..       ..
#
#     4: ..    5: X.    6: .X    7: XX
#        X.       X.       X.       X.
#
#     8: ..    9: X.   10: .X   11: XX
#        .X       .X       .X       .X
#
#    12: ..   13: X.   14: .X   15: XX
#        XX       XX       XX       XX

TILE_SOLID = (

    # outdoors
    15, 12, 15, 14, 13, 15,  0,  0, 15, 15, 15,  8,  4,  7,  3, 11,
    10, 15,  5, 11,  7, 15,  0,  0,  3,  3,  3,  2,  1,  5, 15, 10,
    15,  3, 15, 15, 15, 15,  0,  0, 14, 13,  3, 15,  9, 13, 12, 14,
     0,  0, 15, 15,  0,  0,  0, 15, 15, 15,  0, 15,  6, 15, 15, 15,
    15, 15, 15, 15,  0,  0,  0, 15, 15,  5,  8,  0,  4, 15, 15, 15,
    15, 15, 15, 15,  0,  0,  0,  5, 15,  5, 15,  0, 15, 15, 15, 15,
    15, 15, 15, 14, 13,  0,  0,  5, 15, 15, 15,  0, 15, 15, 15, 15,
    15, 15, 15, 15, 15,  0,  0,  5, 15,  5, 15,  0, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  2,  0,  1, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  8, 15, 15, 15,  4,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0,
    15, 12, 15, 15, 15, 15, 15, 15, 15, 15, 15,  2, 15, 15, 15,  1,
    14, 15, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15,  3, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 12, 15,  5, 10,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

    # indoors
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15,  0, 15, 15, 15, 15,  0, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

    # dungeon
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15,  0, 15, 15, 15, 15,  0, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15,  0,  0,  0, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    
    )
    
with open('../src/generated/tile_solid.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('#include <stdint.h>\n\n')
    f.write('static uint8_t const TILE_SOLID[] PROGMEM =\n{\n')
    for n in range(0, len(TILE_SOLID), 2):
        if n % 32 == 0:
            f.write('   ')
        a = TILE_SOLID[n + 0]
        b = TILE_SOLID[n + 1]
        f.write(' %3d,' % ((a & 15) + (b << 4)))
        if n % 32 >= 30:
            f.write('\n')
    f.write('};\n')
