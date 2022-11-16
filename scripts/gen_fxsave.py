with open('fxsave.bin', 'wb') as f:
    f.write(bytearray([0 for _ in range(4096)]))
