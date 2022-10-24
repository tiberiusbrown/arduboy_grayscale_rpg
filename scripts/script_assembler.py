import re
import sys

strings = []

MAX_STRING_LENGTH = 255

CMD_MSG  = 1
CMD_DLG  = 2
CMD_TMSG = 3
CMD_TDLG = 4
CMD_TP   = 5
CMD_TTP  = 6
CMD_WTP  = 7

def addstring(b, s):
    if len(s) - 2 > MAX_STRING_LENGTH:
        print('String too long: %s' % s)
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

def assemble(s):
    b = []
    s = [x for x in re.findall('\{[^}]*}|\S+', s)]
    i = 0
    while i < len(s):
    
        if s[i] == 'msg':
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
            
        else:
            print('Unknown instruction: "%s"' % s[i])
            sys.exit(1)
    
    return b
