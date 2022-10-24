from extract_sprite_planes import extract

ps = extract('font_masked.png', 8, 8)

def charstr(c):
    if c == 32: return '[space]'
    if c == 92: return '\\\\'
    if c == 127: return '[del]'
    return chr(c)

with open('../src/generated/font_img.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('static constexpr uint8_t FONT_IMG [96 * 8 * 3] PROGMEM =\n{\n   ')
    for i in range(96):
        for j in range(8):
            f.write(' 0x%02x,' % ps[1][i * 8 + j])
            f.write(' 0x%02x,' % ps[2][i * 8 + j])
            f.write(' 0x%02x,' % ps[3][i * 8 + j])
        f.write(' /* %s */\n' % charstr(i + 32))
        if i < 95:
            f.write('   ')
    f.write('};\n')
