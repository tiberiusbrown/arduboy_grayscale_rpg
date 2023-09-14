import sys

ADV = [
    3, 3, 4, 5, 5, 7, 5, 2, 3, 3, 4, 4, 3, 3, 2, 3,
    5, 4, 5, 5, 5, 5, 5, 5, 5, 5, 2, 3, 5, 5, 5, 5,
    8, 5, 5, 5, 6, 5, 5, 6, 5, 2, 4, 5, 5, 6, 5, 6,
    5, 6, 5, 5, 5, 5, 5, 7, 5, 5, 5, 3, 3, 3, 4, 6,
    3, 4, 4, 4, 4, 4, 3, 5, 4, 2, 3, 4, 2, 6, 4, 4,
    4, 4, 3, 4, 3, 4, 4, 6, 4, 5, 4, 3, 2, 3, 5, 4
    ]

ADV_BW = [
    3, 3, 4, 5, 5, 7, 5, 2, 3, 3, 4, 4, 3, 3, 2, 3,
    5, 4, 5, 5, 5, 5, 5, 5, 5, 5, 2, 3, 5, 5, 5, 4,
    8, 5, 5, 5, 5, 5, 5, 5, 5, 2, 4, 5, 5, 6, 5, 5,
    5, 5, 5, 5, 6, 5, 6, 6, 6, 5, 5, 3, 3, 3, 4, 6,
    3, 4, 4, 4, 4, 4, 3, 4, 4, 2, 3, 4, 2, 6, 4, 4,
    4, 4, 3, 4, 3, 4, 4, 6, 4, 4, 4, 4, 2, 4, 5, 4
    ]
    
def wrap(str, w):
    str = list(str)
    i = 0
    x = 0
    while i < len(str):
        t = ord(str[i])
        i += 1
        if t == ord('\n'):
            x = 0
            continue
        x += ADV[t - ord(' ')]
        if x > w:
            i -= 1
            while t != ord(' ') and i != 0:
                i -= 1
                t = ord(str[i])
            if i != 0:
                str[i] = '\n'
    return ''.join(str).split('\n')

def check_wrap(str, w, n):
    r = wrap(str, w)
    if len(r) > n:
        print('String "%s" too long (%d lines intead of %d)' % (str, len(r), n))
        sys.exit(1)
    return r

if __name__ == "__main__":
    with open('../src/generated/font_adv.hpp', 'w') as f:
        f.write('#pragma once\n\n')
        f.write('constexpr uint8_t FONT_ADV[96] PROGMEM =\n{\n')
        for i in range(6):
            f.write('   ')
            for j in range(16):
                f.write(' %d,' % ADV[i*16+j])
            f.write('\n')
        f.write('};\n')
    with open('../src/generated/font_bw_adv.hpp', 'w') as f:
        f.write('#pragma once\n\n')
        f.write('constexpr uint8_t FONT_ADV[96] PROGMEM =\n{\n')
        for i in range(6):
            f.write('   ')
            for j in range(16):
                f.write(' %d,' % ADV_BW[i*16+j])
            f.write('\n')
        f.write('};\n')
