import sys
import csv
from font import check_wrap

def num(x):
    return 0 if x == '' else int(x)

def to_ascii(x):
    return x.replace(chr(8217), "'")
def to_ascii_row(row):
    return [to_ascii(x) for x in row]

with open('items.csv', newline='') as f:
    reader = csv.reader(f, delimiter=',', quotechar='"')
    rows = [to_ascii_row(row) for row in reader]
    rows = rows[1:]
    
names = []

NNAME = 0
NMSG = 0
for t in rows:
    n = len(t[5]) + 1
    if n > NNAME: NNAME = n
    n = len(t[6]) + 1
    if n > NMSG: NMSG = n

N = NNAME + NMSG
with open('../arduboy_build/item_strings.bin', 'wb') as f:
    for t in rows:
        name = t[5]
        msg = t[6]
        check_wrap(name, 100, 1)
        msg = '\n'.join(check_wrap(msg, 128, 2))
        bytes = bytearray([0 for x in range(N)])
        for i in range(len(name)):
            bytes[i] = ord(name[i])
        for i in range(len(msg)):
            bytes[NNAME + i] = ord(msg[i])
        f.write(bytes)

with open('../src/generated/num_items.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('constexpr uint8_t NUM_ITEMS = %d;\n' % len(rows))
    f.write('constexpr uint8_t ITEM_NAME_LEN = %d;\n' % NNAME)
    f.write('constexpr uint8_t ITEM_TOTAL_LEN = %d;\n' % N)

with open('../src/generated/item_info_prog.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('item_info_t const ITEM_INFO[] PROGMEM =\n{\n')
    for t in rows:
        f.write('    { %d, %d, %d, %d, IT_%s },\n' %
            (num(t[0]), num(t[1]), num(t[2]), num(t[3]), t[4].upper()))
    f.write('};\n')

