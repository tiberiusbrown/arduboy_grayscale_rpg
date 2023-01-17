import sys
import csv
import re
from font import check_wrap

def num(x):
    return 0 if x == '' else int(x)
def pnum(x):
    x = num(x)
    return x + 256 if x < 0 else x

def to_name(x):
    return x.split('#')[0]

def to_ascii(x):
    return x.replace(chr(8217), "'")
def to_ascii_row(row):
    return [to_ascii(x) for x in row]

with open('items.csv', newline='') as f:
    reader = csv.reader(f, delimiter=',', quotechar='"')
    rows = [to_ascii_row(row) for row in reader]
    rows = rows[1:]

with open('consumables.csv', newline='') as f:
    reader = csv.reader(f, delimiter=',', quotechar='"')
    consumables = [to_ascii_row(row) for row in reader]
    consumables = consumables[1:]

NUM_ITEMS = len(rows)
names = []
NNAME = 0
NMSG = 0
for t in rows:
    n = len(to_name(t[5])) + 1
    if n > NNAME: NNAME = n
    n = len(t[6]) + 1
    if n > NMSG: NMSG = n
for t in consumables:
    n = len(t[2]) + 1
    if n > NNAME: NNAME = n
    n = len(t[3]) + 1
    if n > NMSG: NMSG = n
N = NNAME + NMSG
#with open('../arduboy_build/item_strings.bin', 'wb') as f:
#    for t in rows:
#        name = t[5]
#        msg = t[6]
#        check_wrap(name, 90, 1)
#        msg = '\n'.join(check_wrap(msg, 124, 2))
#        bytes = bytearray([0 for x in range(N)])
#        for i in range(len(name)):
#            bytes[i] = ord(name[i])
#        for i in range(len(msg)):
#            bytes[NNAME + i] = ord(msg[i])
#        f.write(bytes)
#    for t in consumables:
#        name = t[0]
#        msg = t[1]
#        check_wrap(name, 90, 1)
#        msg = '\n'.join(check_wrap(msg, 124, 2))
#        bytes = bytearray([0 for x in range(N)])
#        for i in range(len(name)):
#            bytes[i] = ord(name[i])
#        for i in range(len(msg)):
#            bytes[NNAME + i] = ord(msg[i])
#        f.write(bytes)
    
with open('../src/generated/num_items.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('constexpr uint8_t NUM_ITEMS = %d;\n' % NUM_ITEMS)
    f.write('constexpr uint8_t ITEM_NAME_LEN = %d;\n' % NNAME)
    f.write('constexpr uint8_t ITEM_DESC_LEN = %d;\n' % (N - NNAME))
    #f.write('constexpr uint8_t ITEM_TOTAL_LEN = %d;\n' % N)
    f.write('constexpr uint8_t NUM_CONSUMABLES = %d;\n' % len(consumables))
    f.write('enum\n{\n')
    for r in consumables:
        x = 'CIT_' + re.sub('[^a-zA-Z]+', '_', r[2])
        f.write('    %s,\n' % x)
    f.write('};\n')
    t = 0
    for i in range(len(consumables)):
        r = consumables[i]
        if int(r[0]) != 0:
            t += (1 << i)
    f.write('constexpr uint8_t CIT_BATTLE_ONLY = %d;\n' % t)
    t = 0
    for i in range(len(consumables)):
        r = consumables[i]
        if int(r[1]) != 0:
            t += (1 << i)
    f.write('constexpr uint8_t CIT_ARDU_ONLY = %d;\n' % t)

TYPES = {
    'weapon' : 0,
    'shield' : 1,
    'armor'  : 2,
    'helm'   : 3,
    'shoes'  : 4,
    'ring'   : 5,
    'amulet' : 6,
    }

with open('../arduboy_build/item_info.bin', 'wb') as f:
    for t in rows:
        bytes = [0 for _ in range(N + 5)]
        bytes[0] = TYPES[t[4].lower()]
        bytes[1] = pnum(t[0])
        bytes[2] = pnum(t[1])
        bytes[3] = pnum(t[2])
        bytes[4] = pnum(t[3])
        name = to_name(t[5])
        check_wrap(name, 90, 1)
        for n in range(len(name)):
            bytes[5+n] = ord(name[n])
        desc = t[6]
        desc = '\n'.join(check_wrap(desc, 124, 2))
        for n in range(len(desc)):
            bytes[5+NNAME+n] = ord(desc[n])
        f.write(bytearray(bytes))
    for t in consumables:
        name = t[2]
        msg = t[3]
        check_wrap(name, 90, 1)
        msg = '\n'.join(check_wrap(msg, 124, 2))
        bytes = bytearray([0 for x in range(N)])
        for i in range(len(name)):
            bytes[i] = ord(name[i])
        for i in range(len(msg)):
            bytes[NNAME + i] = ord(msg[i])
        f.write(bytes)

#with open('../src/generated/item_info_prog.hpp', 'w') as f:
#    f.write('#pragma once\n\n')
#    f.write('item_info_t const ITEM_INFO[] PROGMEM =\n{\n')
#    for t in rows:
#        f.write('    { %d, %d, %d, %d, IT_%s },\n' %
#            (num(t[0]), num(t[1]), num(t[2]), num(t[3]), t[4].upper()))
#    f.write('};\n')

