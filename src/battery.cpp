#include "common.hpp"

battery_info_t bat;

void update_battery()
{
    uint16_t reading = 0;

#ifdef ARDUINO
    static uint16_t v;
    uint16_t f = nframe & 0x03ff;
    if(f == 0)
    {
#if TEST_LIPO_DISCHARGE
        voltage = v;
        if(voltage != 0)
        {
            EEPROM.update(eeprom_addr + 0, uint8_t(voltage >> 0));
            EEPROM.update(eeprom_addr + 1, uint8_t(voltage >> 8));
            eeprom_addr += 2;
        }
#endif
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
        v += (ADC - 200);
        power_adc_disable();
    }
#if TEST_LIPO_DISCHARGE
    if(btns_pressed & BTN_B)
    {
        for(uint16_t i = 16; i < 1024; i += 2)
        {
            uint16_t t = EEPROM.read(i + 0);
            t |= (uint16_t(EEPROM.read(i + 1)) << 8);
            Serial.print(t);
            Serial.print('\n');
        }
    }
#endif
#endif

    if(reading == 0) return;

    // process new reading

    uint16_t old_reading = bat.reading;

    // update filtered reading
    if(old_reading == 0)
    {
        bat.reading = reading;
        return;
    }
    bat.reading = (bat.reading + reading) >> 1;

    // update first derivative history
    for(uint8_t i = 3; i > 0; --i)
        bat.dr[i] = bat.dr[i - 1];
    bat.dr[0] = bat.reading - old_reading;

    for(uint8_t i = 1; i < 4; ++i)
        if(bat.dr[i] == 0) return;

    // update filtered second derivative
    int16_t d2r = bat.dr[0] * 2 + bat.dr[1] - bat.dr[2] - bat.dr[3] * 2;
    if(bat.d2r == 0)
    {
        bat.d2r = d2r * 8;
        return;
    }
    bat.d2r = ((bat.d2r * 7) >> 3) + d2r;

    // low battery decision
    bat.low_battery = (bat.d2r >= 100 && reading >= 6500 && bat.dr[0] < 0);
}
