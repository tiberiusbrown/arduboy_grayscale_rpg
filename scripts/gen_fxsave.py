with open('fxsave.bin', 'wb') as f:
    f.write(bytearray([0xff for _ in range(4096)]))
