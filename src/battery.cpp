#include "common.hpp"

battery_info_t bat;

static int16_t filter256(int16_t y, int16_t x)
{
    // y     : current filter state
    // x     : input
    // return: next filter state
    return y + ((x - y) >> 8);
}

#if RECORD_LIPO_DISCHARGE
#include <hardwareSerial.h>
#include "ArduboyFX.h"
#endif

static void battery_dsp(int16_t reading)
{
    if(bat.stage != 255) ++bat.stage;

    // update reading
    int16_t raw_r = int16_t(reading * 64);
    int16_t old_r = bat.r;
    if(bat.stage == 32)
    {
        bat.r = bat.r32 = raw_r;
    }
    else if(bat.stage > 32)
    {
        bat.r = filter256(bat.r, raw_r);
    }

    // update first derivatives
    int16_t old_dr = bat.dr;
    if(bat.stage == 64)
    {
        int16_t raw_dr = (bat.r - bat.r32) * (256 / 32);
        bat.dr = raw_dr;
    }
    else if(bat.stage > 64)
    {
        int16_t raw_dr = (bat.r - old_r) * 256;
        bat.dr = filter256(bat.dr, raw_dr);
    }

    // update second derivatives
    if(bat.stage == 64)
    {
        bat.ddr = 0;
    }
    else if(bat.stage > 64)
    {
        int16_t raw_ddr = (bat.dr - old_dr) * 512;
        bat.ddr = filter256(bat.ddr, raw_ddr);
    }

    // update low battery decision
    if(bat.stage >= 64)
    {
        bat.low_battery = (bat.dr >= 0 && bat.ddr >= 800);
    }

    bat.raw = reading;

#if RECORD_LIPO_DISCHARGE
    static uint24_t save_addr = 0;
    if(state == STATE_MAP && save_addr < 7200 * 2)
    {
        FX::writeEnable();
        FX::seekCommand(SFC_WRITE, ((uint24_t)(FX::programSavePage) << 8) + save_addr);
        FX::writeByte(uint8_t(reading));
        FX::writeByte(uint8_t(reading >> 8));
        FX::disable();
        save_addr += 2;
    }
#endif
}

void update_battery()
{
    int16_t reading = 0;

    uint16_t f = nframe & 0x3f;

#ifdef ARDUINO
    static int16_t v = 0;
    if(f == 0)
    {
        reading = v;
        v = 0;
    }
    if((f & 0x000f) == 0x0000)
    {
        power_adc_enable();
    }
    if((f & 0x000f) == 0x0001)
    {
        ADCSRA |= _BV(ADSC);
        while(bit_is_set(ADCSRA, ADSC));
        v += (ADC - 300);
        power_adc_disable();
    }
#endif

    if(reading != 0)
        battery_dsp(reading);
}
