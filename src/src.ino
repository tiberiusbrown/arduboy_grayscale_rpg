#define ABG_IMPLEMENTATION
#include "common.hpp"

#include "ArduboyFX.h"
#include "generated/fxdata.h"

decltype(a) a;

#ifndef DEBUG_BUILD
// adapted from Arbuboy2 library
// changes: remove flashlight logic / calls to delayShort
int main() __attribute__ ((OS_main));
int main()
{
    // disable USB
    UDCON = _BV(DETACH);
    UDIEN = 0;
    UDINT = 0;
    USBCON = _BV(FRZCLK);
    UHWCON = 0;
    power_usb_disable();

    init();

    // This would normally be done in the USB code that uses the TX and RX LEDs
    TX_RX_LED_INIT;
    TXLED0;
    RXLED0;

    // Set the DOWN button pin for INPUT_PULLUP
    bitSet(DOWN_BUTTON_PORT, DOWN_BUTTON_BIT);
    bitClear(DOWN_BUTTON_DDR, DOWN_BUTTON_BIT);

    setup();

    for(;;) loop();
}
#endif

static void handle_buttons_and_update()
{
    uint8_t b = btns_down;
    btns_down = a.buttonsState();
    btns_pressed = btns_down & ~b;
    update();
}

void loop()
{
#ifdef DEBUG_MONOCHROME
    abg_detail::current_plane = 1;
    FX::disableOLED();
    handle_buttons_and_update();
    render();
    FX::enableOLED();
    Arduboy2Base::display(CLEAR_BUFFER);
    static uint32_t prev = 0;
    uint32_t curr = millis();
    int16_t diff = curr - prev;
    if(diff < 30)
        delay(30 - diff);
    prev = curr;
#else
    a.waitForNextPlane();
    FX::disableOLED();
    if(a.needsUpdate())
        handle_buttons_and_update();
    render();
    FX::enableOLED();
#endif
}

void setup()
{  

    a.boot();
    
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

#ifdef DEBUGBUILD
    Serial.begin(9600);
#endif
    
    //constexpr uint16_t FX_SAVE_PAGE = (FX_DATA_PAGE - 16) & ~15;
    //FX::begin(FX_DATA_PAGE, FX_SAVE_PAGE);
    //FX::enableOLED();
    
    {
        uint8_t t;
        uint16_t p;
        uint16_t ptr;
        asm volatile(R"ASM(
        
                ldi  %A[ptr], 0x14
                ldi  %B[ptr], 0x00
                ldi  %A[p], lo8(%[DATA_PAGE])
                ldi  %B[p], hi8(%[DATA_PAGE])
                lpm  %[t], %a[ptr]+
                subi %[t], lo8(%[VECTOR_KEY])
                lpm  %[t], %a[ptr]+
                sbci %[t], hi8(%[VECTOR_KEY])
                brne 1f
                lpm  %B[p], %a[ptr]+
                lpm  %A[p], %a[ptr]+
            1:  sts  %[dataPage]+0, %A[p]
                sts  %[dataPage]+1, %B[p]
                
                ;ldi  %A[ptr], 0x18
                ;    %B[ptr] still 0x00
                ldi  %A[p], lo8(%[SAVE_PAGE])
                ldi  %B[p], hi8(%[SAVE_PAGE])
                lpm  %[t], %a[ptr]+
                subi %[t], lo8(%[VECTOR_KEY])
                lpm  %[t], %a[ptr]+
                sbci %[t], hi8(%[VECTOR_KEY])
                brne 2f
                lpm  %B[p], %a[ptr]+
                lpm  %A[p], %a[ptr]+
            2:  sts  %[savePage]+0, %A[p]
                sts  %[savePage]+1, %B[p] 
        
            )ASM"
            : [t]            "=&d" (t)
            , [p]            "=&d" (p)
            , [ptr]          "=&z" (ptr)
            : [dataPage]     ""    (&FX::programDataPage)
            , [savePage]     ""    (&FX::programSavePage)
            , [DATA_PAGE]    "i"   (FX_DATA_PAGE)
            , [SAVE_PAGE]    "i"   (FX_SAVE_PAGE)
            , [VECTOR_KEY]   "i"   (FX_VECTOR_KEY_VALUE)
            );
    }
    
    FX::disableOLED();
    FX::wakeUp();
    initialize();
    FX::enableOLED();
    
#ifndef DEBUG_MONOCHROME
    a.startGray();
#endif
}
