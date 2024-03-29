with open('fxdata-data.bin', 'rb') as f:
    data = f.read()
    
with open('../src/generated/fxdata_emulated.hpp', 'w') as f:
    f.write('#pragma once\n\nuint8_t const FXDATA[] =\n{\n')
    n = 0
    for b in data:
        f.write('%3d,' % int(b))
        if n % 16 == 15:
            f.write('\n')
        n += 1
    f.write('\n};\n')
    