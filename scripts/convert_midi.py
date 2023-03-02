from mido import MidiFile
import sys

NUM_CHANNELS = 2
QUARTER_NOTE_BEATS = 8
END_VOLUME = 0.0

def delay_cmd(beats):
    if beats <= 32: return [64 + beats - 1]
    if beats <= 256: return [128 + 5, beats - 1]
    return [(128 | (1 << 4)) + 5, ((beats - 1) >> 8) & 0xff, (beats - 1) & 0xff]

def byte16(u16):
    return [u16 & 0xff, (u16 >> 8) & 0xff]

def convert(
        fname,
        qnb = QUARTER_NOTE_BEATS,
        ev = END_VOLUME,
        vf = 1.0):
    mid = MidiFile(fname + '.mid')
    if mid.type == 2:
        print('asynchronous midi not supported: "%s"' % fname)
        sys.exit(1)
    
    # collect all notes
    t = 0 # current time
    now_notes = {}         # note -> (time, volume)
    outstanding_notes = {} # note -> (time, volume)
    notes = [] # (note, start, duration, volume)
    for i, track in enumerate(mid.tracks):
        for msg in track:
            if msg.is_meta: continue
            if msg.channel > NUM_CHANNELS - 1: continue
            beats = int(round(msg.time * qnb / 96 / 2))
            if beats > 0:
                for k in now_notes:
                    if k not in outstanding_notes:
                        outstanding_notes[k] = now_notes[k]
                    else:
                        print('Warning (%s): overlapping notes at time %d' % (fname, t))
                now_notes = {}
                t += beats
            if msg.type == 'note_on':
                note = msg.note
                volume = int(msg.velocity * vf)
                if volume <= 0: continue
                if volume > 127: volume = 127
                if note not in now_notes:
                    now_notes[note] = (t, volume)
            if msg.type == 'note_off':
                note = msg.note
                if note in outstanding_notes:
                    start = outstanding_notes[note][0]
                    volume = outstanding_notes[note][1]
                    del outstanding_notes[note]
                    notes.append((note, start, t - start, volume))
    notes = sorted(notes, key=lambda x:(x[1], -x[0]))
    
    # assign notes to channels
    channels = [[] for _ in range(NUM_CHANNELS)]
    times = [0] * NUM_CHANNELS
    for note in notes:
        if note[0] < 36 or note[0] > 98:
            print('Warning (%s): note out of range at time %d' % (fname, note[1]))
            continue
        found = False
        for i in range(NUM_CHANNELS):
            dt = note[1] - times[i]
            if dt < 0: continue
            found = True
            times[i] = note[1] + note[2]
            #add note off and delay if necessary
            if dt > 0:
                channels[i] += [0] # note off
                channels[i] += delay_cmd(dt)
                pass
            # add note and delay
            channels[i] += [0x74, note[3]] # set volume
            slide = int(note[3] * (1 - ev) / note[2])
            channels[i] += [(128 | (1 << 4)) + 1, 0, (256 - slide) & 0xff]
            channels[i] += [note[0] - 35]
            channels[i] += delay_cmd(note[2])
            break
        if not found:
            print('Warning (%s): too many simultaneous notes at time %d' % (fname, note[1]))
    for i in range(NUM_CHANNELS):
        channels[i] += [97]
        
    # generate atmlib byte list
    #bytes = [3]
    #bytes += [NUM_CHANNELS]
    #base = 2 + NUM_CHANNELS * 2 + 1 + NUM_CHANNELS
    #for i in range(NUM_CHANNELS):
    #    bytes += byte16(base)
    #    base += len(channels[i])
    #bytes += [NUM_CHANNELS]
    #for i in range(NUM_CHANNELS):
    #    bytes += [i]
    #for i in range(NUM_CHANNELS):
    #    bytes += channels[i]
    #print(bytes)
    #print(len(bytes))
    
    # generate bindata
    bytes = []
    base = NUM_CHANNELS * 2
    for i in range(NUM_CHANNELS):
        bytes += byte16(base)
        base += len(channels[i])
    for i in range(NUM_CHANNELS):
        bytes += channels[i]
    with open('../arduboy_build/%s.bin' % fname, 'wb') as f:
        f.write(bytearray(bytes))

convert('song_peaceful', ev = 0.5)
convert('song_peaceful2', ev = 0.5, qnb = 8)
convert('song_peaceful3', ev = 0.15, qnb = 8, vf = 2.0)
convert('song_peaceful4', ev = 0.15, qnb = 3, vf = 2.0)
convert('song_victory', qnb = 2)
convert('song_defeat', qnb = 8)

