#define ABG_IMPLEMENTATION
#include "common.hpp"

#include "ArduboyFX.h"
#include "generated/fxdata.h"

decltype(a) a;

ARDUBOY_NO_USB

#define DISABLE_TIMER_0 1

uint8_t updown_frames;

void loop()
{
    a.idle();
    if(!a.nextFrame())
        return;
    FX::disableOLED();
    if(a.needsUpdate())
    {
        uint8_t b = btns_down;
        btns_down = a.buttonsState();
        btns_pressed = btns_down & ~b;
#if DISABLE_TIMER_0
        if((btns_down & (BTN_UP | BTN_DOWN)) == (BTN_UP | BTN_DOWN))
            ++updown_frames;
        else
            updown_frames = 0;
        if(updown_frames == 45)
            a.exitToBootloader();
#endif
        update();
    }
    render();
    FX::enableOLED();
}

void setup()
{
    a.boot();
    FX::begin(FX_DATA_PAGE, 0);
    FX::enableOLED();
    initialize();
#if DISABLE_TIMER_0
    bitWrite(TIMSK0, TOIE0, 0);
#endif
    a.startGray();
}
