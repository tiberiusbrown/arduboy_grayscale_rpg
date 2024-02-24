from PIL import Image

def get_shade(rgba, shades, shade):
    w = (254 + shades) // shades
    b = (shade + 1) * w
    return 1 if rgba[0] >= b else 0

def get_mask(rgba):
    return 1 if rgba[3] >= 128 else 0

def convert_impl(fname, shades, sw = None, sh = None, num = None):

    if not (shades >= 2 and shades <= 4):
        print('shades argument must be 2, 3, or 4')
        return None

    print('Converting:', fname)

    im = Image.open(fname).convert('RGBA')
    pixels = list(im.getdata())
    
    masked = False
    for i in pixels:
        if i[3] < 255:
            masked = True
            break
    
    w = im.width
    h = im.height
    if sw is None: sw = w
    if sh is None: sh = h
    nw = w // sw
    nh = h // sh
    if num is None: num = nw * nh
    sp = (sh + 7) // 8
    
    if nw * nh <= 0:
        print('%s: Invalid sprite dimensions' % fname)
        return None
        
    bytes = bytearray([sw, sh])
    
    for n in range(num):
        bx = (n % nw) * sw
        by = (n // nw) * sh
        for shade in range(shades - 1):
            for p in range(sp):
                for ix in range(sw):
                    x = bx + ix
                    byte = 0
                    mask = 0
                    for iy in range(8):
                        y = p * 8 + iy
                        if y >= sh: break
                        y += by
                        i = y * w + x
                        rgba = pixels[i]
                        byte |= (get_shade(rgba, shades, shade) << iy)
                        mask |= (get_mask(rgba) << iy)
                    bytes += bytearray([byte])
                    if masked:
                        bytes += bytearray([mask])
    
    return bytes
    
def convert_header(fname, fout, sym, shades, sw = None, sh = None, num = None):
    bytes = convert_impl(fname, shades, sw, sh, num)
    if bytes is None: return
    with open(fout, 'w') as f:
        f.write('#pragma once\n\n#include <stdint.h>\n\n')
        f.write('#ifdef ARDUINO_ARCH_AVR\n')
        f.write('#include <avr/pgmspace.h>\n')
        f.write('#else\n')
        f.write('#define PROGMEM\n')
        f.write('#endif\n\n')
        f.write('constexpr uint8_t %s[] PROGMEM =\n{\n' % sym)
        for n in range(len(bytes)):
            if n % 16 == 0:
                f.write('    ')
            f.write('%3d,' % bytes[n])
            f.write(' ' if n % 16 != 15 else '\n')
        if len(bytes) % 16 != 0:
            f.write('\n')
        f.write('};\n')

def convert_bin(fname, fout, shades, sw = None, sh = None, num = None):
    bytes = convert_impl(fname, shades, sw, sh, num)
    if bytes is None: return
    with open(fout, 'wb') as f:
        f.write(bytes)

def convert(fout, sym, fname, sw, sh, num = None):
    if fout[-4:] == '.bin':
        convert_bin(fname, fout, 4, sw, sh, num)
    else:
        convert_header(fname, fout, sym, 4, sw, sh, num)

BASE = '../src/generated/'
BINBASE = '../arduboy_build/'
DEMO_BASE = '../demo/'

bw = '_bw'
bw = ''

convert(BASE + 'font_img.hpp', 'FONT_IMG', 'font.png', 8, 8);
convert_header('font_bw.png', BASE + 'font_bw_img.hpp', 'FONT_IMG', 2, 8, 8);

convert(DEMO_BASE + 'tile_img.hpp', 'TILE_IMG', 'tiles.png', 16, 16, 224)
convert(BINBASE + 'tile_img.bin', '', 'tiles.png', 16, 16)

#convert(BASE + 'rounded_borders_white_img.hpp', 'ROUNDED_BORDERS_WHITE_IMG_PROG', 'rounded_borders_white.png', 3, 8)
#convert(BASE + 'rounded_borders_black_img.hpp', 'ROUNDED_BORDERS_BLACK_IMG_PROG', 'rounded_borders_black.png', 2, 8)

convert(BINBASE + 'portrait_img.bin', '', 'portraits.png', 32, 32)
convert(BINBASE + 'player_img.bin', '', 'player_sprites.png', 16, 16)
convert(BINBASE + 'sprites_img.bin', '', 'sprites.png', 16, 16)
#convert(BINBASE + 'asleep_img.bin', '', 'asleep.png', 16, 16)
convert(BINBASE + 'battle_menu_chain_img.bin', '', 'battle_menu_chain.png', 3, 8)
convert(BINBASE + 'battle_menu_img.bin', '', 'battle_menu' + bw + '.png', 32, 24)
convert(BINBASE + 'battle_attacker_img.bin', '', 'battle_attacker.png', 9, 8)
convert(BINBASE + 'battle_arrow_img.bin', '', 'battle_arrow.png', 9, 8)
convert(BINBASE + 'battle_banners_img.bin', '', 'battle_banners.png', 144, 16)
convert(BINBASE + 'battle_select_img.bin', '', 'battle_select.png', 22, 24)
convert(BINBASE + 'battle_alert_img.bin', '', 'battle_alert.png', 13, 16)
convert(BINBASE + 'battle_zz_img.bin', '', 'battle_zz.png', 7, 8)
convert(BINBASE + 'battle_order_img.bin', '', 'battle_order' + bw + '.png', 6, 8)
convert(BINBASE + 'game_over_img.bin', '', 'game_over.png', 128, 64)
convert(BINBASE + 'title_img.bin', '', 'title.png', 128, 64)
convert(BINBASE + 'battery_img.bin', '', 'battery.png', 10, 8)
convert(BINBASE + 'pause_menu_img.bin', '', 'pause_menu.png', 128, 16)
convert(BINBASE + 'quit_img.bin', '', 'quit.png', 128, 64)
convert(BINBASE + 'options_img.bin', '', 'options.png', 128, 64)
convert(BINBASE + 'check_img.bin', '', 'check.png', 8, 8)
convert(BINBASE + 'slider_img.bin', '', 'slider.png', 7, 8)
convert(BINBASE + 'arrows_img.bin', '', 'arrows.png', 8, 8)
#convert(BINBASE + 'party_members_img.bin', '', 'party_members.png', 128, 16)
convert(BINBASE + 'a_items_img.bin', '', 'a_items' + bw + '.png', 30, 8)
convert(BINBASE + 'ap_img.bin', '', 'ap.png', 4, 8)
convert(BINBASE + 'innates_img.bin', '', 'innates' + bw + '.png', 128, 56)
convert(BINBASE + 'item_cats_img.bin', '', 'item_cats.png', 62, 16)
convert(BINBASE + 'no_items_img.bin', '', 'no_items.png', 128, 64)
convert(BINBASE + 'got_item_img.bin', '', 'got_item.png', 62, 16)
convert(BINBASE + 'title_masked_img.bin', '', 'title_masked.png', 114, 16)
convert(BINBASE + 'press_a_img.bin', '', 'press_a.png', 34, 16)
convert(BINBASE + 'objective_arrows_img.bin', '', 'objective_arrows.png', 16, 16)
convert(BINBASE + 'world_img.bin', '', 'world.png', 128, 64)
convert(BINBASE + 'audio_img.bin', '', 'audio.png', 11, 8)
