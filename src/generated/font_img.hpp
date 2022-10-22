#pragma once

static constexpr uint8_t FONT_IMG [96 * 8 * 3] PROGMEM =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* [space] */
    0x2f, 0x2f, 0x2f, 0x08, 0x27, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ! */
    0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* " */
    0x14, 0x3c, 0x3c, 0x3c, 0x17, 0x3f, 0x14, 0x3f, 0x3f, 0x03, 0x14, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* # */
    0x00, 0x37, 0x37, 0x25, 0x7f, 0x7f, 0x25, 0x3d, 0x3d, 0x01, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* $ */
    0x00, 0x07, 0x07, 0x15, 0x27, 0x37, 0x21, 0x1e, 0x3f, 0x00, 0x3f, 0x3f, 0x29, 0x38, 0x39, 0x28, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* % */
    0x02, 0x38, 0x3a, 0x35, 0x2f, 0x3f, 0x09, 0x3f, 0x3f, 0x12, 0x38, 0x3a, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* & */
    0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ' */
    0x7e, 0x3c, 0x7e, 0x34, 0xc3, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ( */
    0x02, 0xc1, 0xc3, 0x3c, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ) */
    0x00, 0x06, 0x06, 0x02, 0x07, 0x07, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* * */
    0x00, 0x08, 0x08, 0x08, 0x1c, 0x1c, 0x1c, 0x08, 0x1c, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* + */
    0xc0, 0xa0, 0xe0, 0xe0, 0x60, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* , */
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* - */
    0x20, 0x20, 0x20, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* . */
    0x3a, 0x3c, 0x3e, 0x17, 0x0f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* / */
    0x21, 0x1e, 0x3f, 0x33, 0x21, 0x33, 0x2d, 0x33, 0x3f, 0x12, 0x0c, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0 */
    0x00, 0x02, 0x02, 0x3c, 0x03, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 1 */
    0x21, 0x32, 0x33, 0x23, 0x39, 0x3b, 0x21, 0x2f, 0x2f, 0x05, 0x22, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 2 */
    0x20, 0x11, 0x31, 0x31, 0x25, 0x35, 0x26, 0x3f, 0x3f, 0x0a, 0x10, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 3 */
    0x14, 0x18, 0x1c, 0x11, 0x16, 0x17, 0x3f, 0x3f, 0x3f, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 4 */
    0x27, 0x17, 0x37, 0x37, 0x25, 0x37, 0x25, 0x3d, 0x3d, 0x05, 0x18, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 5 */
    0x29, 0x1e, 0x3f, 0x3d, 0x27, 0x3f, 0x25, 0x3d, 0x3d, 0x01, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 6 */
    0x01, 0x01, 0x01, 0x05, 0x39, 0x3d, 0x09, 0x07, 0x0f, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 7 */
    0x04, 0x3b, 0x3f, 0x3d, 0x27, 0x3f, 0x25, 0x3f, 0x3f, 0x02, 0x18, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8 */
    0x00, 0x2f, 0x2f, 0x2f, 0x29, 0x2f, 0x09, 0x3f, 0x3f, 0x12, 0x0c, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 9 */
    0x24, 0x24, 0x24, 0x00, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* : */
    0xc4, 0xa4, 0xe4, 0xe0, 0x64, 0xe4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ; */
    0x00, 0x1c, 0x1c, 0x08, 0x1c, 0x1c, 0x14, 0x1c, 0x1c, 0x08, 0x14, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* < */
    0x00, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* = */
    0x08, 0x14, 0x1c, 0x14, 0x1c, 0x1c, 0x08, 0x1c, 0x1c, 0x14, 0x08, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* > */
    0x01, 0x02, 0x03, 0x07, 0x29, 0x2f, 0x29, 0x07, 0x2f, 0x05, 0x02, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ? */
    0x42, 0x3c, 0x7e, 0x43, 0xfe, 0xff, 0xe1, 0xbf, 0xff, 0x8f, 0xb5, 0xbf, 0xe1, 0xbf, 0xff, 0x13, 0xfe, 0xff, 0x5c, 0x00, 0x5c, 0x00, 0x00, 0x00, /* @ */
    0x20, 0x00, 0x20, 0x14, 0x38, 0x3c, 0x18, 0x17, 0x1f, 0x10, 0x1f, 0x1f, 0x24, 0x38, 0x3c, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* A */
    0x3f, 0x3f, 0x3f, 0x3f, 0x25, 0x3f, 0x3f, 0x25, 0x3f, 0x1a, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* B */
    0x2d, 0x1e, 0x3f, 0x25, 0x33, 0x37, 0x33, 0x21, 0x33, 0x00, 0x33, 0x33, 0x00, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* C */
    0x3f, 0x3f, 0x3f, 0x3f, 0x21, 0x3f, 0x33, 0x21, 0x33, 0x12, 0x3f, 0x3f, 0x12, 0x0c, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* D */
    0x00, 0x3f, 0x3f, 0x3f, 0x25, 0x3f, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* E */
    0x00, 0x3f, 0x3f, 0x3f, 0x05, 0x3f, 0x05, 0x05, 0x05, 0x01, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* F */
    0x0c, 0x1e, 0x1e, 0x0c, 0x33, 0x3f, 0x33, 0x29, 0x3b, 0x39, 0x2b, 0x3b, 0x29, 0x1a, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* G */
    0x3f, 0x3f, 0x3f, 0x3f, 0x04, 0x3f, 0x04, 0x04, 0x04, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* H */
    0x3f, 0x3f, 0x3f, 0x00, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* I */
    0x00, 0x30, 0x30, 0x30, 0x20, 0x30, 0x1e, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* J */
    0x3f, 0x3f, 0x3f, 0x37, 0x0c, 0x3f, 0x0b, 0x1e, 0x1f, 0x2b, 0x31, 0x3b, 0x21, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* K */
    0x3f, 0x3f, 0x3f, 0x3f, 0x20, 0x3f, 0x20, 0x20, 0x20, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* L */
    0x3e, 0x3f, 0x3f, 0x34, 0x0e, 0x3e, 0x0c, 0x38, 0x3c, 0x34, 0x0e, 0x3e, 0x3e, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* M */
    0x3f, 0x3f, 0x3f, 0x3a, 0x07, 0x3f, 0x0a, 0x1c, 0x1e, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* N */
    0x00, 0x1e, 0x1e, 0x0c, 0x33, 0x3f, 0x33, 0x21, 0x33, 0x21, 0x33, 0x33, 0x25, 0x1e, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* O */
    0x00, 0x3f, 0x3f, 0x3f, 0x09, 0x3f, 0x0d, 0x09, 0x0d, 0x06, 0x0f, 0x0f, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* P */
    0x00, 0x1e, 0x1e, 0x00, 0x33, 0x33, 0x23, 0x31, 0x33, 0x01, 0x33, 0x33, 0x05, 0x3e, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Q */
    0x3f, 0x3f, 0x3f, 0x3f, 0x05, 0x3f, 0x15, 0x0d, 0x1d, 0x11, 0x3f, 0x3f, 0x03, 0x20, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* R */
    0x25, 0x12, 0x37, 0x2d, 0x37, 0x3f, 0x3b, 0x2d, 0x3f, 0x10, 0x3b, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* S */
    0x01, 0x01, 0x01, 0x01, 0x3f, 0x3f, 0x01, 0x3f, 0x3f, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* T */
    0x3f, 0x1f, 0x3f, 0x2f, 0x30, 0x3f, 0x30, 0x20, 0x30, 0x1f, 0x3f, 0x3f, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* U */
    0x07, 0x07, 0x07, 0x1a, 0x3c, 0x3e, 0x04, 0x38, 0x3c, 0x03, 0x0f, 0x0f, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* V */
    0x16, 0x0f, 0x1f, 0x0c, 0x38, 0x3c, 0x20, 0x1f, 0x3f, 0x20, 0x1f, 0x3f, 0x08, 0x38, 0x38, 0x16, 0x0f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* W */
    0x12, 0x21, 0x33, 0x20, 0x1f, 0x3f, 0x0c, 0x1e, 0x1e, 0x28, 0x33, 0x3b, 0x21, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* X */
    0x03, 0x01, 0x03, 0x35, 0x0e, 0x3f, 0x04, 0x3e, 0x3e, 0x04, 0x03, 0x07, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Y */
    0x20, 0x31, 0x31, 0x2d, 0x39, 0x3d, 0x29, 0x27, 0x2f, 0x21, 0x23, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Z */
    0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* [ */
    0x17, 0x0f, 0x1f, 0x3a, 0x3c, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* \\ */
    0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ] */
    0x00, 0x06, 0x06, 0x01, 0x03, 0x03, 0x01, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ^ */
    0x40, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* _ */
    0x02, 0x01, 0x03, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ` */
    0x10, 0x3c, 0x3c, 0x3c, 0x2c, 0x3c, 0x0c, 0x3c, 0x3c, 0x1c, 0x20, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a */
    0x3f, 0x3f, 0x3f, 0x3c, 0x24, 0x3c, 0x24, 0x3c, 0x3c, 0x04, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* b */
    0x18, 0x3c, 0x3c, 0x3c, 0x24, 0x3c, 0x14, 0x24, 0x34, 0x34, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* c */
    0x18, 0x3c, 0x3c, 0x3c, 0x24, 0x3c, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* d */
    0x18, 0x3c, 0x3c, 0x3c, 0x2c, 0x3c, 0x2c, 0x2c, 0x2c, 0x24, 0x08, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* e */
    0x3f, 0x3e, 0x3f, 0x3d, 0x07, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* f */
    0x18, 0xbc, 0xbc, 0xbc, 0xa4, 0xbc, 0xbc, 0xfc, 0xfc, 0x7c, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* g */
    0x3f, 0x3f, 0x3f, 0x00, 0x0c, 0x0c, 0x38, 0x3c, 0x3c, 0x3c, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* h */
    0x3d, 0x3d, 0x3d, 0x3d, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* i */
    0x00, 0x80, 0x80, 0xfd, 0xfd, 0xfd, 0x7d, 0x00, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* j */
    0x00, 0x3f, 0x3f, 0x0c, 0x18, 0x1c, 0x08, 0x34, 0x3c, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* k */
    0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* l */
    0x3c, 0x3c, 0x3c, 0x0c, 0x04, 0x0c, 0x38, 0x3c, 0x3c, 0x34, 0x0c, 0x3c, 0x38, 0x3c, 0x3c, 0x38, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* m */
    0x3c, 0x3c, 0x3c, 0x0c, 0x04, 0x0c, 0x38, 0x3c, 0x3c, 0x3c, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* n */
    0x18, 0x3c, 0x3c, 0x3c, 0x24, 0x3c, 0x18, 0x3c, 0x3c, 0x24, 0x18, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* o */
    0xfc, 0xfc, 0xfc, 0x3c, 0x24, 0x3c, 0x24, 0x3c, 0x3c, 0x04, 0x18, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* p */
    0x00, 0x3c, 0x3c, 0x3c, 0x24, 0x3c, 0xfc, 0xfc, 0xfc, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* q */
    0x3c, 0x3c, 0x3c, 0x04, 0x0c, 0x0c, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* r */
    0x04, 0x28, 0x2c, 0x2c, 0x2c, 0x2c, 0x1c, 0x34, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* s */
    0x1f, 0x3f, 0x3f, 0x3f, 0x24, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* t */
    0x1c, 0x3c, 0x3c, 0x30, 0x20, 0x30, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* u */
    0x1c, 0x1c, 0x1c, 0x08, 0x30, 0x38, 0x3c, 0x1c, 0x3c, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* v */
    0x2c, 0x1c, 0x3c, 0x04, 0x38, 0x3c, 0x2c, 0x1c, 0x3c, 0x08, 0x30, 0x38, 0x1c, 0x0c, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* w */
    0x18, 0x24, 0x3c, 0x38, 0x1c, 0x3c, 0x08, 0x34, 0x3c, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* x */
    0x14, 0x8c, 0x9c, 0x34, 0xf8, 0xfc, 0x28, 0x1c, 0x3c, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* y */
    0x24, 0x34, 0x34, 0x24, 0x3c, 0x3c, 0x24, 0x2c, 0x2c, 0x24, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* z */
    0x6b, 0x1c, 0x7f, 0x89, 0xf7, 0xff, 0x81, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* { */
    0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* | */
    0xb5, 0xc3, 0xf7, 0x89, 0x7e, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* } */
    0x08, 0x06, 0x0e, 0x02, 0x06, 0x06, 0x06, 0x04, 0x06, 0x01, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ~ */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* [del] */
};
