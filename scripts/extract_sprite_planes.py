from PIL import Image

def extract(fname, sw, sh, num = 0):

    im = Image.open(fname)
    
    if num == 0:
        num = im.width * im.height // (sw * sh)

    sbytes = sw * sh // 8
    p0 = [0] * (num * sbytes)
    p1 = [0] * (num * sbytes)
    masked = False
    if 'transparency' in im.info:
        masked = True
        pm = [0] * (num * sbytes)

    index = 0
    for i in range(num):
        r = (i // (im.width // sw)) * sh
        c = (i % (im.width // sw)) * sw
        for ir in range(sh // 8):
            for ic in range(sw):
                b0 = 0
                b1 = 0
                bm = 0
                for tr in range(8):
                    x = im.getpixel((c + ic, r + ir * 8 + 7 - tr))
                    if masked:
                        bm = (bm << 1)
                        if x != 0:
                            bm = bm | 1
                            x = x - 1
                    x0 = x & 1
                    x1 = (x >> 1) & 1
                    b0 = (b0 << 1) | x0
                    b1 = (b1 << 1) | x1
                p0[index] = b0
                p1[index] = b1
                if masked:
                    pm[index] = bm
                index = index + 1
        
    if masked:
        return [num, p0, p1, pm]
    else:
        return [num, p0, p1]
    