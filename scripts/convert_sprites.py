from extract_sprite_planes import extract

def convert(fout, sym, fname, sw, sh, num = 0, start = 0):
    ps = extract(fname, sw, sh, num, start)
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

convert(BASE + 'tile_img.hpp', 'TILE_IMG_PROG', 'tiles.png', 16, 16, 64, 0)
convert(BASE + 'rounded_borders_white_img.hpp', 'ROUNDED_BORDERS_WHITE_IMG_PROG', 'rounded_borders_white.png', 3, 8)
convert(BASE + 'rounded_borders_black_img.hpp', 'ROUNDED_BORDERS_BLACK_IMG_PROG', 'rounded_borders_black.png', 2, 8)
convert(BINBASE + 'tile_img.bin', '', 'tiles.png', 16, 16, 192, 64)

convert(BINBASE + 'portrait_img.bin', '', 'portraits.png', 32, 32)
convert(BINBASE + 'player_img.bin', '', 'player_sprites.png', 16, 16)
convert(BINBASE + 'sprites_img.bin', '', 'sprites.png', 16, 16)
convert(BINBASE + 'asleep_img.bin', '', 'asleep.png', 16, 16)
convert(BINBASE + 'battle_menu_chain_img.bin', '', 'battle_menu_chain.png', 3, 8)
convert(BINBASE + 'battle_menu_img.bin', '', 'battle_menu.png', 32, 40)
convert(BINBASE + 'battle_attacker_img.bin', '', 'battle_attacker.png', 9, 8)
convert(BINBASE + 'battle_arrow_img.bin', '', 'battle_arrow.png', 9, 8)
convert(BINBASE + 'battle_banners_img.bin', '', 'battle_banners.png', 144, 16)
convert(BINBASE + 'battle_select_img.bin', '', 'battle_select.png', 22, 24)
convert(BINBASE + 'battle_alert_img.bin', '', 'battle_alert.png', 13, 16)
convert(BINBASE + 'game_over_img.bin', '', 'game_over.png', 128, 64)
convert(BINBASE + 'title_img.bin', '', 'title.png', 128, 64)
