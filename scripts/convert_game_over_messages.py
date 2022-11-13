import sys

N = 128

with open('game_over_messages.txt', 'r') as f:
    msgs = f.readlines()
    
num = 0
with open('../arduboy_build/game_over_messages.bin', 'wb') as f:
    for msg in msgs:
        if msg[-1] == '\n':
            msg = msg[:-1]
        if len(msg) == 0:
            continue
        if num >= 256:
            break
        num += 1
        if len(msg) > N - 1:
            print('Message too long: "%s"' % msg)
            sys.exit(1)
        bytes = bytearray([0 for x in range(N)])
        for i in range(len(msg)):
            bytes[i] = ord(msg[i])
        f.write(bytes)

with open('../src/generated/num_game_over_messages.hpp', 'w') as f:
    f.write('#pragma once\n\nconstexpr uint8_t NUM_GAME_OVER_MESSAGES = %d;\n' % num)
