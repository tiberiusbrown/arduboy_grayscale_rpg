# ATMlib2

ATMlib stands for **Arduboy Tracker Music** and is based on [_**Squawk**_](https://github.com/stg/Squawk "Squawk Github Page") a minimalistic 8-bit software synthesizer & playroutine library for Arduino, created by Davey Taylor aka STG.

While _Squawk_ provides a very nice synth, it wasn't optimized for a small footprint. Songs are not very efficient in size, so Joeri Gantois aka JO3RI asked Davey to help him work on a new score format and so ATMlib was born.

ATMlib2 is built on the work of JO3RI and Davey and adds a lot of new exciting features! ATMlib2 is not compatible with ATMlib music scores.

Contributors (Alphabetical order):

* Davey Taylor - ATMsynth - Effects
* Delio Brignoli
* Jay Garcia (Modus Create)
* Joeri Gantois - Effects

Thanks to [Modus Create](https://moduscreate.com) for sponsoring and participating in the development.

### New Features

* Updated score format for simpler and smaller decoder
* Add reduced size score formats: minimal scores have 1 byte overhead (for the header)
* Command encoding designed for simpler and smaller decoder
* Macros for manual song creation
* C++ API built on top of C API
* 4 channels: runtime selectable waveform can be square with programmable duty cycle or noise
* LFO and slide effects can be applied to square wave duty cycle
* Asynchronous playback of 1 sound effect as a mono music score on an arbitrary channel with independent tempo. Music is muted on the channel used by sound effects and resumed when the sound effect is stopped or playback finishes
* Dual timer design: 93kHz PWM carrier for improved audio quality (was 31kHz)
* 10bit PWM resolution: mixing of 4 8bit full volume channels without clipping
* Reduced CPU usage by interrupts: interrupt rate halved without reducing sample rate, simplified interrupt handler
* Note frequencies are closer to the expected value within the limits of the MCU's clocksource
* Effects can selectively disabled at compile time to reduce code size
* Automatic interrupt and PWM disable when not in use to save CPU cycles and battery

All of the above while code size was reduced (from 3245 to 2268 bytes as of da3cdbca79ea5ce22431061a4365d45d771434b3) and bug were squashed!

### To Do

* Reduce SRAM used by effect parameters
* Try implementing selectable waveforms: triangle, sawtooth, square and lookup table
* Support 8 channels music

### Music playback example


##### C
``` C
...
#include "atm_synth.h"

void setup() {
    arduboy.begin();
    arduboy.setFrameRate(15);
    arduboy.audio.on();
    
    /* Begin playback of song */
    atm_synth_setup();
    atm_synth_play_score(score);
}
```

##### C++
``` C++
...
#include "ATMlib.h"

ATMsynth ATM;

void setup() {
    arduboy.begin();
    arduboy.setFrameRate(15);
    arduboy.audio.on();

    /* Begin playback of song */
    ATM.play(score);
}
```


### Sound effect playback example

``` C
...
#include "atm_synth.h"

/* Temporary storage used by sound effect while playing back */
struct atm_sfx_state sfx_state;

void setup() {
    arduboy.begin();
    arduboy.setFrameRate(15);
    arduboy.audio.on();

    // Begin playback of song.
    atm_synth_setup();
    atm_synth_play_score(score);
}

void loop() {
    if (!(arduboy.nextFrame()))
        return;

    arduboy.pollButtons();

    if (arduboy.justPressed(B_BUTTON)) {
        /* Start playback of sfx1 on channel 1 */
        atm_synth_play_sfx_track(OSC_CH_ONE, &sfx1, &sfx_state);
    }

    if (arduboy.justPressed(A_BUTTON)) {
        atm_synth_stop_sfx_track(&sfx_state);
    }

    arduboy.display();
}
```

### Music score memory layout

Music scores have a single byte header describing the format of the following bytes. The smallest possible score layout is _single pattern, mono_ and has 1 byte header plus N bytes for the pattern. The shortest possible score that still produces audio looks like this (5 bytes total):

``` C
const PROGMEM struct sfx_data {
    uint8_t fmt;
    uint8_t pattern0[4];
} sfx1 = {
    .fmt = ATM_SCORE_FMT_MINIMAL_MONO,
    .pattern0 = {
        /* Use default tempo */
        /* Volume must be set because it defaults to 0 */
        ATM_CMD_M_SET_VOLUME(31),
        ATM_CMD_I_NOTE_C4,
        ATM_CMD_M_DELAY_TICKS(25),
        ATM_CMD_I_STOP,
    },
};
```

#### Overview

A score is made up of a common header followed by _chunks_ concatenated one after the other with no padding. The common header has a fixed size of one byte and is always present. Chunks are ordered as follows:

| Order  | Name            | Optional
|--------|-----------------|--------------
| 1      | Common header   | N
| 2      | Pattern info    | Y
| 3      | Channel info    | Y
| 4      | Extensions      | Y
| 5      | Patterns data   | N


#### Common header

```-``` means reserved bits. They should be ignored and written as zero.

| Offset | Size         | Name
|--------|--------------|--------------
| 0      | 1            | Format ID/Version

```
b------oc    :   Format ID/Version
 ||||||||
 |||||||└->  0   c: channel info chunk present flag
 ||||||└-->  1   o: pattern info chunk present flag
 └└└└└└--->  7:2 [reserved]

```

#### Pattern offsets information

This chunk is present if flag ```o``` is set in the common header. 

| Offset | Size              | Name
|--------|-------------------|--------------
| 0      | 1                 | Pattern count
| 1      | 2*[pattern count] | Pattern offsets


```
b--pppppp   : pattern count, p: number of patterns
-------------------------------------------------------------------
uint16_t[p] : pattern offsets, array of p elements, each element P
              is the offset in bytes of the start of pattern P
              from the beginning of the score data
              (including the common header).
```

#### Channel information

This chunk is present if flag ```c``` is set in the common header. 

| Offset | Size              | Name
|--------|-------------------|--------------
| 0      | 1                 | Channel count
| 1      | [channel count]   | Entry patterns

```
0b------cc   : channel count, c: number of channels
uint8_t[c]   : entry patterns | array of c elements, each element
               uint8_t[c_i] is the index of the first pattern played by
               channel c_i.
```

#### Reserved for extensions

If the ```c``` flag is set in the header, extensions can be located immediately after pattern info (or immediately after channel info in case it is present) and occupy space up to the beginning of pattern data. When there are no extensions (none is defined for now) pattern data can immediately follow the previous block.

#### Pattern data

When the ```o``` flag is set in the header each pattern's start offset is specified in the pattern information chunk. When the ```o``` flag is not set pattern data follows immediately the end of the channel information chunk, if present, otherwise it follows the common header.


| ```o``` and ```c``` flags | Pattern data location
|---------------------------|-------------------
| o:0, c:0                  | Immediately after the header. Offset: 1 byte
| o:0, c:1                  | Immediately after channel information block: 2+[number of channels] bytes
| o:1, c:0                  | Specified by offset array
| o:1, c:1                  | Specified by offset array

### Commands encoding

Commands are of two types: immediate when they have no extra parameters and parametrised when they can be followed by parameter bytes. Command bit-space is partitioned as follows:

```
00nnnnnn : 1-63 note ON number, 0 note OFF (Immediate)
010ddddd : Delay d+1 ticks (max 32 ticks) (Immediate)
0110iiii : i = command ID (16 commands) (Immediate)
0111kkkk : k = single byte parameter command ID (16 commands)
1ssscccc : s+1 = parameters byte count (s=7 reserved), c = command ID (16 commands)
1111---- : [reserved]
```

#### Immediate command Macros

```
ATM_CMD_I_TRANSPOSITION_OFF - Transpose OFF
ATM_CMD_I_PATTERN_END       - Pattern End/Return
ATM_CMD_I_GLISSANDO_OFF     - Glissando/Portamento OFF
ATM_CMD_I_ARPEGGIO_OFF      - Arpeggio OFF
ATM_CMD_I_NOTECUT_OFF       - Note Cut OFF (alias for Arpeggio OFF)
```

#### Single byte command Macros (do not use directly, see the following Public Command Macros section)

```
ATM_CMD_1P_SET_TRANSPOSITION - Set transposition
ATM_CMD_1P_ADD_TRANSPOSITION - Add transposition
ATM_CMD_1P_SET_TEMPO         - Set tempo
ATM_CMD_1P_ADD_TEMPO         - Add tempo
ATM_CMD_1P_SET_VOLUME        - Set Volume
ATM_CMD_1P_SET_WAVEFORM      - Set waveform
ATM_CMD_1P_SET_MOD           - Set square wave duty cycle
```

#### N bytes command Macros (do not use directly, see the following Public Command Macros section)

N bytes commands use the lower nibble to encode 16 command IDs and bits 6:4 to encode the number of parameter bytes which follow the command starting at 0 i.e. a value of ```b10000000``` means parametrised command ID 0 followed by 1 parameter byte.

```
ATM_CMD_NP_SET_LOOP_PATTERN - Setup pattern loop
ATM_CMD_NP_SLIDE            - Slide FX
ATM_CMD_NP_CALL             - Call
ATM_CMD_NP_GLISSANDO_ON     - Glissando/Portamento
ATM_CMD_NP_ARPEGGIO_ON      - Arpeggio/Note Cut
ATM_CMD_NP_LONG_DELAY       - Long delay
ATM_CMD_NP_LFO              - LFO FX
```

#### Public Command Macros

Updates to command encoding, parsing and effects implementation is still in progress and subject to change without notice. Command macros listed below are provided as a way to mitigate issues caused by changes and are the only approved tool to write patterns for now.

##### Glissando

```
Glissando - Raise or lower the current note by one semitone every N ticks

Macros         : ATM_CMD_M_GLISSANDO_ON(p1)
               : ATM_CMD_I_GLISSANDO_OFF

Parameter count: 1

P1
    Size   : 1 byte
    Name   : Effect configuration
    Format : bdnnnnnnn
              |└└└└└└└-> ticks between each note change minus one
              └--------> 1: shift pitch down, 0: shift pitch up
```

##### Arpeggio

```
Arpeggio - Play a second and optionally third note after each played note

Macros         : ATM_CMD_M_ARPEGGIO_ON(p1, p2)
               : ATM_CMD_I_ARPEGGIO_OFF

Parameter count: 2

P1
    Size   : 1 byte
    Name   : Chord note shifts
    Format : bkkkknnnn
              ||||└└└└-> 3nd note shift: semitones to add to the 2nd [0,14]
              └└└└-----> 2nd note shift: semitones to add to the 1st [0,14]

P2
    Size   : 1 byte
    Name   : Effect configuration
    Format : b-edttttt
              |||└└└└└-> ticks between note change minus one [0, 31]
              ||└------> 0: auto repeat, 1: trigger with note
              |└-------> 1: skip 3rd note, 0: play 3rd note
              └--------> [reserved]
```

##### Note Cut

```
Note cut - Stop note automatically after N ticks

Macros         : ATM_CMD_M_NOTECUT_ON(p1)
               : ATM_CMD_I_NOTECUT_OFF

Parameter count: 1

P1
    Size   : 1 byte
    Name   : Effect configuration
    Format : b--dttttt
              |||└└└└└-> ticks between note change minus one [0, 31]
              ||└------> 0: auto repeat, 1: trigger with note
              |└-------> [reserved]
              └--------> [reserved]
```

##### Set waveform

```
Set waveform - Set waveform for a pattern

Macros         : ATM_CMD_M_SET_WAVEFORM(p1)

Parameter count: 1

P1
    Size   : 1 byte
    Name   : Waveform type
    Format : b-------w
              |||||||└-> 0: square, 1: noise
              └└└└└└└--> [reserved]
```

##### Slide FX

```
Slide/Slide advanced - Ramp oscillator parameter up or down

Macros         : ATM_CMD_M_SLIDE_VOL_ON(p1)
               : ATM_CMD_M_SLIDE_FREQ_ON(p1)
               : ATM_CMD_M_SLIDE_MOD_ON(p1)
               : ATM_CMD_M_SLIDE_VOL_ADV_ON(p1, p2)
               : ATM_CMD_M_SLIDE_FREQ_ADV_ON(p1, p2)
               : ATM_CMD_M_SLIDE_MOD_ADV_ON(p1, p2)
               : ATM_CMD_M_SLIDE_VOL_OFF
               : ATM_CMD_M_SLIDE_FREQ_OFF
               : ATM_CMD_M_SLIDE_MOD_OFF

Parameter count: 1/2/3

P0         : Implicit in macro name
    Size   : 1 byte
    Name   : Oscillator parameter to slide
    Note   : When only this parameter is present the effect is turned off
    Format : b------pp
              ||||||└└-> Oscillator parameter 0:vol, 1:freq, 3:mod
              └└└└└└---> [reserved]

P1
    Size   : 1 byte
    Name   : Amount to slide up or down per tick (or each N ticks)
    Range  : [-128:127] (i8)

P2
    Size   : 1 byte
    Name   : Configuration
    Note   : Defaults to 0 when not present i.e. update every tick, clamp and keep fading
    Format : bornnnnnn
              ||└└└└└└-> Ticks between update minus one
              |└-------> Retrigger flag 1: restart effect on note-on 0: keep sliding on note-on
              └--------> Overflow flag 1: let overflow, 0: clamp
```

##### LFO FX

```
LFO - Low frequency oscillator applied to one of the synth parameters

Macros         : ATM_CMD_M_TREMOLO_ON(depth, rate)
               : ATM_CMD_M_VIBRATO_ON(depth, rate)
               : ATM_CMD_M_MOD_LFO_ON(depth, rate)
               : ATM_CMD_M_TREMOLO_OFF
               : ATM_CMD_M_VIBRATO_OFF
               : ATM_CMD_M_MOD_LFO_OFF

Parameter count: 1/2/3

P0         : Implicit in macro name
    Size   : 1 byte
    Name   : Oscillator parameter to modulate
    Note   : When only this parameter is present the effect is turned off
    Format : b------pp
              ||||||└└-> Oscillator parameter 0:vol, 1:freq, 3:mod
              └└└└└└---> [reserved]

P1
    Size   : 1 byte
    Name   : LFO depth
    Format : b-ddddddd
              |└└└└└└└-> Oscillator parameter delta per tick
              └--------> [reserved]

P2
    Size   : 1 byte
    Name   : Configuration
    Note   : Defaults to 0 when not present (update every tick)
    Format : b---nnnnn
              |||└└└└└-> Ticks between update minus one
              └└└------> [reserved]
```

##### Call

```
Call/Call Repeat - jump to a pattern index and optionally repeat it N times

Macros         : ATM_CMD_M_CALL(pattern_index)
               : ATM_CMD_M_CALL_REPEAT(pattern_index, repeat_count)
               : ATM_CMD_I_PATTERN_END

Parameter count: 1/2

P1
    Size  : 1 byte
    Name  : Pattern index to jump to
    Range : [0:255] (u8)

P2
    Size  : 1 byte
    Name  : Repeat times - 1
    Range : [0:255] (u8)
    Note  : Default to 0 when not present (play once)
```


##### Long delay

```
Long Delay - delay any number of ticks between 1 and 65534

Macros         : ATM_CMD_M_DELAY_TICKS(delay_up_to_32_ticks)
               : ATM_CMD_M_DELAY_TICKS_1(delay_up_to_256_ticks)
               : ATM_CMD_M_DELAY_TICKS_2(delay_up_to_65534_ticks)

Parameter count: 1

P1
    Size  : 1/2 bytes
    Name  : Delay value
    Range : [0:255] (u8) or [0:65533] (u16)
    Note  : 1 <= delay <= 256 is enconded with one byte as (delay-1),
            delay > 256 is encoded as an uint16_t as (delay-1) using
            native CPU endiannes (i.e. high byte first)
```

##### Set transposition

```
Set transposition - set transposition in semitones

Macros         : ATM_CMD_M_SET_TRANSPOSITION(semitones)
               : ATM_CMD_I_TRANSPOSITION_OFF

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Semitones
    Range : [-128:127] (i8)
```

##### Add transposition

```
Add transposition - add to current transposition in semitones

Macros         : ATM_CMD_M_ADD_TRANSPOSITION(semitones)
               : ATM_CMD_I_TRANSPOSITION_OFF

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Semitones
    Range : [-128:127] (i8)
```


##### Set tempo

```
Set tempo - set tick rate in Hz

Macros         : ATM_CMD_M_SET_TEMPO(tick_hz)

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Tick rate in Hz 
    Range : [8:255] (u8)
```

##### Add tempo

```
Add tempo - add to current tick rate in Hz

Macros         : ATM_CMD_M_ADD_TEMPO(tick_hz)

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Tick rate in Hz to add or subtract
    Range : [-128:127] (i8)
```

##### Set Volume

```
Set volume - set volume

Macros         : ATM_CMD_M_SET_VOLUME(volume)

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Volume
    Format : b-ddddddd
              |└└└└└└└-> Volume [0:127], defaults to 0
              └--------> [reserved]
```

##### Set square wave duty cycle

```
Set square wave duty cycle - set square wave duty cycle

Macros         : ATM_CMD_M_SET_MOD(duty_cycle)

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Duty cycle
    Range : [0:255] (u8)
    Note  : Values from 0 to 255 map to 0%/100% duty cycle, the default 0x7F is 50%
```

##### Setup pattern loop

```
Setup pattern loop - set the pattern index to loop to

Macros         : ATM_CMD_M_SET_LOOP_PATTERN(loop_pattern)

Parameter count: 1

P1
    Size  : 1 byte
    Name  : Pattern index to loop to when the score finishes
    Range : [0:255] (u8)
    Note  : The loop index takes effect when all channels have stopped
```
