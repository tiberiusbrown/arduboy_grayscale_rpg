#define ABG_IMPLEMENTATION
#define ABG_UPDATE_EVERY_N_DEFAULT 2
#include "common.hpp"

#include "ArduboyFX.h"
#include "generated/fxdata.h"

decltype(a) a;

void loop()
{
    if(!a.nextFrame())
        return;
    FX::disableOLED();
    if(a.needsUpdate())
    {
        uint8_t b = btns_down;
        btns_down = a.buttonsState();
        btns_pressed = btns_down & ~b;
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
    a.startGray();
}
