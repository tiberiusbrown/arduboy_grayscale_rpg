import sys

with open('game_over_messages.txt', 'r') as f:
    msgs = f.readlines()
    
N = 0
for i in range(len(msgs)):
    if msgs[i][-1] == '\n':
        msgs[i] = msgs[i][:-1]
    if len(msgs[i]) >= N:
        N = len(msgs[i]) + 1

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
        bytes = bytearray([0 for x in range(N)])
        for i in range(len(msg)):
            bytes[i] = ord(msg[i])
        f.write(bytes)

with open('../src/generated/num_game_over_messages.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write('constexpr uint8_t NUM_GAME_OVER_MESSAGES = %d;\n' % num)
    f.write('constexpr uint8_t GAME_OVER_MESSAGE_LEN = %d;\n' % N)
