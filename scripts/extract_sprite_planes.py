from PIL import Image

def extract(fname, sw, sh, triplane = False, num = 0, start = 0):

    im = Image.open(fname)
    
    if num == 0:
        num = im.width * im.height // (sw * sh) - start

    sbytes = sw * sh // 8
    p0 = [0] * (num * sbytes)
    p1 = [0] * (num * sbytes)
    p2 = [0] * (num * sbytes)
    masked = False
    if 'transparency' in im.info:
        masked = True
        pm = [0] * (num * sbytes)
        
    im = im.convert('RGBA')

    index = 0
    for i in range(start, start + num):
        r = (i // (im.width // sw)) * sh
        c = (i % (im.width // sw)) * sw
        for ir in range(sh // 8):
            for ic in range(sw):
                b0 = 0
                b1 = 0
                b2 = 0
                bm = 0
                for tr in range(8):
                    rgba = im.getpixel((c + ic, r + ir * 8 + 7 - tr))
                    x = 0
                    if rgba[0] > 0xc0:
                        x = 3
                    elif rgba[0] > 0x80:
                        x = 2
                    elif rgba[0] > 0x40:
                        x = 1
                    if masked:
                        x += 1
                    if rgba[3] == 0:
                        x = 0
                    if masked:
                        bm = (bm << 1)
                        if x != 0:
                            bm = bm | 1
                            x = x - 1
                    if triplane:
                        x0 = 1 if x >= 1 and x <= 3 else 0
                        x1 = 1 if x >= 2 and x <= 3 else 0
                        x2 = 1 if x >= 3 and x <= 3 else 0
                    else:
                        x0 = x & 1
                        x1 = (x >> 1) & 1
                        x2 = 0
                    b0 = (b0 << 1) | x0
                    b1 = (b1 << 1) | x1
                    b2 = (b2 << 1) | x2
                p0[index] = b0
                p1[index] = b1
                p2[index] = b2
                if masked:
                    pm[index] = bm
                index = index + 1
        
    if masked:
        return [num, p0, p1, p2, pm]
    else:
        return [num, p0, p1, p2]
    