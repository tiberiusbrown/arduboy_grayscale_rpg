from extract_sprite_planes import extract

def convert(fout, sym, fname, sw, sh, num = 0):
    ps = extract(fname, sw, sh, num)
    sbytes = sw * sh // 8
    if fout[-4:] == '.hpp':
        with open(fout, "w") as f:
            f.write('#pragma once\n\n')
            f.write('static constexpr uint8_t %s[] PROGMEM =\n{\n' % sym)
            f.write('    %d, %d,%s\n' % (sw, sh, ' /* (with mask) */' if len(ps) > 3 else ''))
            for i in range(ps[0]):
                f.write('\n    /* frame %d */\n   ' % i)
                if len(ps) > 3:
                    for j in range(sbytes):
                        f.write(' 0x%02x,' % ps[1][i * sbytes + j])
                        f.write(' 0x%02x,' % ps[2][i * sbytes + j])
                        f.write(' 0x%02x,' % ps[3][i * sbytes + j])
                else:
                    for j in range(sbytes):
                        f.write(' 0x%02x,' % ps[1][i * sbytes + j])
                    f.write('\n   ')
                    for j in range(sw * sh // 8):
                        f.write(' 0x%02x,' % ps[2][i * sbytes + j])
                f.write('\n')
                if i < ps[0] - 1:
                    f.write('   ')
            f.write('};\n')
    if fout[-4:] == '.bin':
        with open(fout, 'wb') as f:
            for i in range(ps[0]):
                if len(ps) > 3:
                    for j in range(sbytes):
                        f.write(bytearray([ps[1][i * sbytes + j]]))
                        f.write(bytearray([ps[3][i * sbytes + j]]))
                    for j in range(sbytes):
                        f.write(bytearray([ps[2][i * sbytes + j]]))
                        f.write(bytearray([ps[3][i * sbytes + j]]))
                else:
                    f.write(bytearray(ps[1][i*sbytes:(i+1)*sbytes]))
                    f.write(bytearray(ps[2][i*sbytes:(i+1)*sbytes]))
                    

BASE = '../src/generated/'
BINBASE = '../arduboy_build/'

convert(BINBASE + 'portrait_img.bin', 'PORTRAIT_IMG', 'portraits.png', 32, 32)
convert(BINBASE + 'player_img.bin', 'PLAYER_IMG', 'player_sprites.png', 16, 16)
convert(BINBASE + 'enemy_img.bin', 'ENEMY_IMG', 'enemy_sprites.png', 16, 16)
convert(BINBASE + 'tile_img.bin', 'TILE_IMG', 'tiles.png', 16, 16)
