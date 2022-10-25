import re
import sys

strings = []
flags = {}

MAX_STRING_LENGTH = 253

CMD_END  =  0

CMD_MSG  =  1
CMD_DLG  =  2
CMD_TMSG =  3
CMD_TDLG =  4
CMD_TP   =  5
CMD_TTP  =  6
CMD_WTP  =  7

CMD_ADD  =  8
CMD_ADDI =  9
CMD_SUB  = 10
CMD_FS   = 11

CMD_JMP  = 12
CMD_BRZ  = 13
CMD_BRN  = 14
CMD_BRFS = 15
CMD_BRFC = 16
CMD_BRNT = 17
CMD_BRNW = 18

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
    
def addflag(b, s):
    if s[0] != '!':
        print('Expected flag, got "%s"' % s)
        sys.exit(1)
    if s not in flags:
        n = len(flags)
        flags[s] = n
    append16(b, flags[s])

def assemble(s):
    b = []
    s = [x for x in re.findall('\{[^}]*}|\S+', s)]
    i = 0
    while i < len(s):
    
        if s[i] == 'end':
            b.append(CMD_END); i += 1
    
        elif s[i] == 'msg':
            b.append(CMD_MSG); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tmsg':
            b.append(CMD_TMSG); i += 1
            b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'dlg':
            b.append(CMD_DLG); i += 1
            b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tdlg':
            b.append(CMD_TDLG); i += 1
            b.append(int(s[i])); i += 1
            b.append(int(s[i])); i += 1
            addstring(b, s[i]); i += 1
            
        elif s[i] == 'tp':
            b.append(CMD_TP); i += 1
            append16(b, s[i]); i += 1
            append16(b, s[i]); i += 1
            
        elif s[i] == 'ttp':
            b.append(CMD_TTP); i += 1
            b.append(int(s[i])); i += 1
            append16(b, s[i]); i += 1
            append16(b, s[i]); i += 1
            
        elif s[i] == 'wtp':
            b.append(CMD_WTP); i += 1
            b.append(int(s[i])); i += 1
            append16(b, s[i]); i += 1
            append16(b, s[i]); i += 1
            
        elif s[i] == 'add':
            b.append(CMD_ADD); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            
        elif s[i] == 'addi':
            b.append(CMD_ADDI); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            b.append(int(s[i])); i += 1
            
        elif s[i] == 'sub':
            b.append(CMD_SUB); i += 1
            b.append(dstreg(s[i]) + (reg(s[i+1])<<4)); i += 2
            
        elif s[i] == 'fs':
            b.append(CMD_FS); i += 1
            addflag(b, s[i]); i += 1
            
        elif s[i] == 'jmp':
            b.append(CMD_JMP); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brz':
            b.append(CMD_BRZ); i += 1
            b.append(reg(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brn':
            b.append(CMD_BRN); i += 1
            b.append(reg(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brfs':
            b.append(CMD_BRFS); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brfc':
            b.append(CMD_BRFC); i += 1
            addflag(b, s[i]); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brnt':
            b.append(CMD_BRNT); i += 1
            b.append(int(s[i])); i += 1
            b.append(s[i]); i += 1
            
        elif s[i] == 'brnw':
            b.append(CMD_BRNW); i += 1
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
                b[i] = labels[b[i]]
            else:
                print('Unknown label: "%s"' % b[i])
                sys.exit(1)
    
    return b
