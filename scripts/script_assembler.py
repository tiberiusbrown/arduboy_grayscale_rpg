import re
import sys
import csv
from font import wrap

MAX_STRING_LENGTH = 219
MAX_NAME_LENGTH = 18

portraits = {}
sprites = {}
enemies = {}

# battle flags used
bats = {}

tiles = {
    'T_OutdoorSwitchOff'   : 51  & 255,
    'T_OutdoorSwitchOn'    : 50  & 255,
    'T_DungeonSwitchOff'   : 544 & 255,
    'T_DungeonSwitchOn'    : 528 & 255,
    'T_DungeonFloor'       : 530 & 255,
    'T_DungeonMagicFloor'  : 564 & 255,
    'T_DungeonMagicFloor2' : 565 & 255,
    'T_OutdoorStairs'      : 58  & 255,
    'T_IndoorStairs'       : 279 & 255,
    'T_DungeonStairs'      : 535 & 255,
    'T_DungeonBarsOpen'    : 550 & 255,
    'T_DungeonBarsClosed'  : 551 & 255,
    'T_DungeonButtonOff'   : 549 & 255,
    'T_DungeonButtonOn'    : 548 & 255,
    'T_OutdoorButtonOff'   : 49  & 255,
    'T_OutdoorButtonOn'    : 48  & 255,
    'T_OutdoorRock'        : 55  & 255,
    'T_Grass1'             : 6   & 255,
    'T_Grass2'             : 7   & 255,
    'T_Grass3'             : 22  & 255,
    'T_Grass4'             : 23  & 255,
    }

def id_helper(x):
    return re.sub('[^a-zA-Z]+', '_', x)

def init():
    with open('sprites.csv', newline='') as f:
        reader = csv.reader(f, delimiter=',', quotechar='"')
        for row in reader:
            sprites['S_' + id_helper(row[1])] = int(row[0])
    with open('enemies.csv', newline='') as f:
        reader = csv.reader(f, delimiter=',', quotechar='"')
        for row in reader:
            enemies['E_' + id_helper(row[1])] = int(row[0])    
    with open('portraits.csv', newline='') as f:
        reader = csv.reader(f, delimiter=',', quotechar='"')
        id = 0
        with open('../arduboy_build/portrait_strings.bin', 'wb') as fo:
            for row in reader:
                pn = row[1]
                if len(pn) > MAX_NAME_LENGTH:
                    print('Portrait name too long: "%s"' % row[1])
                    sys.exit(1)
                portraits['P_' + id_helper(pn)] = id
                id += 1
                bytes = [0] * (MAX_NAME_LENGTH + 2)
                bytes[0] = int(row[0])
                for n in range(len(pn)):
                    bytes[n + 1] = ord(pn[n])
                fo.write(bytearray(bytes))

strings = []
stringdict = {}
flags = {}

with open('items.csv', newline='') as f:
    reader = csv.reader(f, delimiter=',', quotechar='"')
    items = [row[5] for row in reader]
    items = items[1:]
for i in range(len(items)):
    items[i] = re.sub('[^a-zA-Z0-9]+', '_', items[i].split('|')[0].strip())
for i in items:
    flags['!ITEM_' + i] = len(flags)

from enum import IntEnum, auto

class AutoNumber(IntEnum):
    def __new__(self):
        value = len(self.__members__)
        obj = int.__new__(self)
        obj._value_ = value
        return obj

class CMD(AutoNumber):

    END  = ()

    MSG  = ()
    TMSG = ()
    DLG  = ()
    TDLG = ()
    BAT  = ()
    EBAT = ()
    TP   = ()
    TTP  = ()
    WTP  = ()
    DIE  = ()

    ADD  = ()
    ADDI = ()
    SUB  = ()
    ANDI = ()

    FS   = ()
    FC   = ()
    FT   = ()
    EP   = ()
    EPF  = ()
    ST   = ()
    STF  = ()
    PA   = ()
    OBJ  = ()
    HEAL = ()
    SOLVED = ()

    JMP  = ()
    BZ   = ()
    BNZ  = ()
    BNEQ = ()
    BGEQ = ()
    BFS  = ()
    BFC  = ()
    BNST = ()
    BNWT = ()
    BNWE = ()
    BNSE = ()
    BNI  = ()
    BNAI = ()
    BPF  = ()

def stringid(s):
    s = s[1:-1]
    if s in stringdict:
        return stringdict[s]
    if len(s) > MAX_STRING_LENGTH:
        print('String too long: %s' % s)
        print('Length: %d' % len(s))
        print('Max:    %d' % MAX_STRING_LENGTH)
        sys.exit(1)
    x = len(strings)
    stringdict[s] = x
    s = '\n'.join(wrap(s, 128))
    for c in s:
        strings.append(ord(c))
    strings.append(0)
    return x

# TODO: merge duplicate strings
def addstring(b, s):
    x = stringid(s)
    b.append((x >> 0) % 256)
    b.append((x >> 8) % 256)

def append16(b, x):
    x = int(x)
    b.append((x >> 0) % 256)
    b.append((x >> 8) % 256)

def reg(s):
    if s[0] != 'r':
        print('Expected register but got "%s"' % s)
        sys.exit(1)
    x = int(s[1:])
    if x < 0 or x >= 16:
        print('Register must be r0 to r15: "%s"' % s)
        sys.exit(1)
    return x

def dstreg(s):
    x = reg(s)
    if x == 0:
        print('r0 is not allowed to be written to')
        sys.exit(1)
    return x
    
def flag(s):
    if s is None:
        s = 'MF_' + str(len(flags))
    elif s[0] != '!':
        print('Expected flag, got "%s"' % s)
        sys.exit(1)
    if s not in flags:
        if s.startswith('ITEM_'):
            print('Undefined item flag: "%s"' % s)
            sys.exit(1)
        n = len(flags)
        flags[s] = n
    return flags[s]
    
def addflag(b, s):
    append16(b, flag(s))

def batflag(b, s, chunk):
    f = flag(s)
    if f in bats:
        print('Duplicate battle flags: "%s" at %s and %s' % (s, str(bats[f]), str(chunk)))
        sys.exit(1)
    bats[f] = chunk
    addflag(b, s)

def addpath(b, s, eps):
    if s not in eps:
        print('Path "%s" not found' % s)
        sys.exit(1)
    b += eps[s]

def tile(s):
    if s not in tiles:
        return int(s)
    return tiles[s]

def sprite(s):
    if s not in sprites:
        print('Sprite not found: "%s"' % s)
        print('Sprites: ', sprites)
        sys.exit(1)
    return sprites[s]
    
def addsprite(b, s):
    b.append(sprite(s))

def addportrait(b, s):
    if s not in portraits:
        print('Portrait not found: "%s"' % s)
        print('Portraits: ', portraits)
        sys.exit(1)
    b.append(portraits[s])

def enemy(s):
    if s == '-': return 255
    if s not in enemies:
        print('Enemy not found: "%s"' % s)
        print('Enemies: ', enemies)
        sys.exit(1)
    return enemies[s]

def addenemy(b, s):
    b.append(enemy(s))

def assemble(s, eps, chunk):
    b = []
    s = [x for x in re.findall('\{[^}]*}|\S+', s)]
    i = 0
    while i < len(s):
    
        if s[i] == 'end':
            b.append(CMD.END); i += 1
    
        elif s[i] == 'msg':
            b.append(CMD.MSG); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tmsg':
            b.append(CMD.TMSG); i += 1
            b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'dlg':
            b.append(CMD.DLG); i += 1
            addportrait(b, s[i]); i += 1
            #b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tdlg':
            b.append(CMD.TDLG); i += 1
            b.append(int(s[i])); i += 1
            addportrait(b, s[i]); i += 1
            #b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'bat':
            b.append(CMD.BAT); i += 1
            batflag(b, s[i], chunk); i += 1
            addenemy(b, s[i]); i += 1
            addenemy(b, s[i]); i += 1
            addenemy(b, s[i]); i += 1
            addenemy(b, s[i]); i += 1
            
        elif s[i] == 'ebat':
            b.append(CMD.EBAT); i += 1
            batflag(b, s[i], chunk); i += 1
            addenemy(b, s[i]); i += 1
            addenemy(b, s[i]); i += 1
            addenemy(b, s[i]); i += 1
            addenemy(b, s[i]); i += 1
            
        elif s[i] == 'tp':
            b.append(CMD.TP); i += 1
            #append16(b, s[i]); i += 1
            #append16(b, s[i]); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'ttp':
            b.append(CMD.TTP); i += 1
            b.append(int(s[i])); i += 1
            #append16(b, s[i]); i += 1
            #append16(b, s[i]); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'wtp':
            b.append(CMD.WTP); i += 1
            b.append(int(s[i])); i += 1
            #append16(b, s[i]); i += 1
            #append16(b, s[i]); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'die':
            b.append(CMD.DIE); i += 1
            
        elif s[i] == 'add':
            b.append(CMD.ADD); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            
        elif s[i] == 'addi':
            b.append(CMD.ADDI); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            b.append(int(s[i], 0)); i += 1
            
        elif s[i] == 'sub':
            b.append(CMD.SUB); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            
        elif s[i] == 'andi':
            b.append(CMD.ANDI); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            b.append(int(s[i], 0)); i += 1
            
        elif s[i] == 'fs':
            b.append(CMD.FS); i += 1
            addflag(b, s[i]); i += 1
            
        elif s[i] == 'fc':
            b.append(CMD.FC); i += 1
            addflag(b, s[i]); i += 1
            
        elif s[i] == 'ft':
            b.append(CMD.FT); i += 1
            addflag(b, s[i]); i += 1
        
        elif s[i] == 'ep':
            b.append(CMD.EP); i += 1
            addsprite(b, s[i]); i += 1
            #b.append(int(s[i])); i += 1
            addpath(b, s[i], eps); i += 1
            
        elif s[i] == 'epf':
            b.append(CMD.EPF); i += 1
            addflag(b, s[i]); i += 1
            addsprite(b, s[i]); i += 1
            #b.append(int(s[i])); i += 1
            addpath(b, s[i], eps); i += 1
            
        elif s[i] == 'st':
            b.append(CMD.ST); i += 1
            b.append(int(s[i])); i += 1
            b.append(tile(s[i])); i += 1
            
        elif s[i] == 'stf':
            b.append(CMD.STF); i += 1
            b.append(int(s[i])); i += 1
            addflag(b, s[i]); i += 1
            b.append(tile(s[i])); i += 1
            
        elif s[i] == 'pa':
            b.append(CMD.PA); i += 1
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'obj':
            b.append(CMD.OBJ); i += 1
            #append16(b, s[i]); i += 1
            #append16(b, s[i]); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
        
        elif s[i] == 'heal':
            b.append(CMD.HEAL); i += 1
        
        elif s[i] == 'solved':
            b.append(CMD.SOLVED); i += 1
            
        elif s[i] == 'jmp':
            b.append(CMD.JMP); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bz':
            b.append(CMD.BZ); i += 1
            b.append(reg(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bnz':
            b.append(CMD.BNZ); i += 1
            b.append(reg(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bneq':
            b.append(CMD.BNEQ); i += 1
            b.append(reg(s[i])); i += 1
            b.append(int(s[i], 0)); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bgeq':
            b.append(CMD.BGEQ); i += 1
            b.append(reg(s[i])); i += 1
            b.append(int(s[i], 0)); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bfs':
            b.append(CMD.BFS); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bfc':
            b.append(CMD.BFC); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bnst':
            b.append(CMD.BNST); i += 1
            b.append(int(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bnwt':
            b.append(CMD.BNWT); i += 1
            b.append(int(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bnwe':
            b.append(CMD.BNWE); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bnse':
            b.append(CMD.BNSE); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bni':
            b.append(CMD.BNI); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bnai':
            b.append(CMD.BNAI); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'bpf':
            b.append(CMD.BPF); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'CHEST':
            i += 1
            t = int(s[i]); i += 1
            f = s[i]; i += 1
            b.append(CMD.BNST)
            b.append(t)
            b.append(3)
            b.append(CMD.FS)
            addflag(b, f)
            b.append(CMD.BFC)
            addflag(b, f)
            b.append(3)
            b.append(CMD.ST)
            b.append(t)
            b.append(82)
            
        elif s[i][-1] == ':':
            b.append(s[i]); i += 1 # hold onto label for now
            
        else:
            print('Unknown instruction: "%s"' % s[i])
            sys.exit(1)
    
    # postprocess labels
    labels = {}
    i = 0
    while i < len(b):
        if isinstance(b[i], str):
            if b[i][-1] == ':':
                labels[b[i][:-1]] = i
                b.pop(i)
                continue
        i += 1
    for i in range(len(b)):
        if isinstance(b[i], str):
            if b[i] in labels:
                b[i] = labels[b[i]] - (i + 1)
            else:
                print('Unknown label: "%s"' % b[i])
                sys.exit(1)
    
    #print(b)
    
    # check valid byte values
    for i in range(len(b)):
        if isinstance(b[i], CMD):
            b[i] = b[i]._value_
        if b[i] < 0:
            b[i] += 256
        if b[i] >= 256:
            print('Value out of range: %d' % x)
            sys.exit(1)

    return b
