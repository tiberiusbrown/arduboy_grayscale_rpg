import re
import sys

strings = []
flags = {}

MAX_STRING_LENGTH = 253

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
    DLG  = ()
    TMSG = ()
    TDLG = ()
    TP   = ()
    TTP  = ()
    WTP  = ()

    ADD  = ()
    ADDI = ()
    SUB  = ()

    FS   = ()
    ST   = ()

    JMP  = ()
    BRZ  = ()
    BRN  = ()
    BRFS = ()
    BRFC = ()
    BRNT = ()
    BRNW = ()

# TODO: merge duplicate strings
def addstring(b, s):
    if len(s) > MAX_STRING_LENGTH:
        print('String too long: %s' % s)
        print('Length: %d' % len(s))
        print('Max:    %d' % MAX_STRING_LENGTH)
        sys.exit(1)
    x = len(strings)
    for c in s[1:-1]:
        strings.append(ord(c))
    strings.append(0)
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
        n = len(flags)
        flags[s] = n
    return flags[s]
    
def addflag(b, s):
    append16(b, flag(s))

def assemble(s):
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
            b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tdlg':
            b.append(CMD.TDLG); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tp':
            b.append(CMD.TP); i += 1
            append16(b, s[i]); i += 1
            append16(b, s[i]); i += 1
            
        elif s[i] == 'ttp':
            b.append(CMD.TTP); i += 1
            b.append(int(s[i])); i += 1
            append16(b, s[i]); i += 1
            append16(b, s[i]); i += 1
            
        elif s[i] == 'wtp':
            b.append(CMD.WTP); i += 1
            b.append(int(s[i])); i += 1
            append16(b, s[i]); i += 1
            append16(b, s[i]); i += 1
            
        elif s[i] == 'add':
            b.append(CMD.ADD); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            
        elif s[i] == 'addi':
            b.append(CMD.ADDI); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'sub':
            b.append(CMD.SUB); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            
        elif s[i] == 'fs':
            b.append(CMD.FS); i += 1
            addflag(b, s[i]); i += 1
            
        elif s[i] == 'st':
            b.append(CMD.ST); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'jmp':
            b.append(CMD.JMP); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brz':
            b.append(CMD.BRZ); i += 1
            b.append(reg(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brn':
            b.append(CMD.BRN); i += 1
            b.append(reg(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brfs':
            b.append(CMD.BRFS); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brfc':
            b.append(CMD.BRFC); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brnt':
            b.append(CMD.BRNT); i += 1
            b.append(int(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brnw':
            b.append(CMD.BRNW); i += 1
            b.append(int(s[i])); i += 1
            b.append(s[i]); i += 1
            
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
