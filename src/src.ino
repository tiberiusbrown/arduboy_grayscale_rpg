#define ABG_IMPLEMENTATION
#include "common.hpp"

#include "ArduboyFX.h"
#include "generated/fxdata.h"

decltype(a) a;

//#ifdef REMOVE_TIMER0
//ISR(remove_timer0, __attribute__((naked))) {}
//#endif

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

static void custom_init()
{
    // this needs to be called before setup() or some functions won't
	// work there
	sei();
    
#ifndef REMOVE_TIMER0
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
#if defined(TCCR0A) && defined(WGM01)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);
#endif

	// set timer 0 prescale factor to 64
#if defined(__AVR_ATmega128__)
	// CPU specific: different values for the ATmega128
	sbi(TCCR0, CS02);
#elif defined(TCCR0) && defined(CS01) && defined(CS00)
	// this combination is for the standard atmega8
	sbi(TCCR0, CS01);
	sbi(TCCR0, CS00);
#elif defined(TCCR0B) && defined(CS01) && defined(CS00)
	// this combination is for the standard 168/328/1280/2560
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);
#elif defined(TCCR0A) && defined(CS01) && defined(CS00)
	// this combination is for the __AVR_ATmega645__ series
	sbi(TCCR0A, CS01);
	sbi(TCCR0A, CS00);
#else
	#error Timer 0 prescale factor 64 not set correctly
#endif

	// enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
	sbi(TIMSK, TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
	sbi(TIMSK0, TOIE0);
#else
	#error	Timer 0 overflow interrupt not set correctly
#endif
#endif
	
    ADCSRA = (_BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADEN));
    
#if 0
	// set a2d prescaler so we are inside the desired 50-200 KHz range.
	#if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
		sbi(ADCSRA, ADPS2);
		sbi(ADCSRA, ADPS1);
		sbi(ADCSRA, ADPS0);
	#elif F_CPU >= 8000000 // 8 MHz / 64 = 125 KHz
		sbi(ADCSRA, ADPS2);
		sbi(ADCSRA, ADPS1);
		cbi(ADCSRA, ADPS0);
	#elif F_CPU >= 4000000 // 4 MHz / 32 = 125 KHz
		sbi(ADCSRA, ADPS2);
		cbi(ADCSRA, ADPS1);
		sbi(ADCSRA, ADPS0);
	#elif F_CPU >= 2000000 // 2 MHz / 16 = 125 KHz
		sbi(ADCSRA, ADPS2);
		cbi(ADCSRA, ADPS1);
		cbi(ADCSRA, ADPS0);
	#elif F_CPU >= 1000000 // 1 MHz / 8 = 125 KHz
		cbi(ADCSRA, ADPS2);
		sbi(ADCSRA, ADPS1);
		sbi(ADCSRA, ADPS0);
	#else // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
		cbi(ADCSRA, ADPS2); 
		cbi(ADCSRA, ADPS1);
		sbi(ADCSRA, ADPS0);
	#endif
	// enable a2d conversions
	sbi(ADCSRA, ADEN);
#endif

	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
#if defined(UCSRB)
	UCSRB = 0;
#elif defined(UCSR0B)
	UCSR0B = 0;
#endif
}

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

    custom_init();

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

uint8_t updown_frames;

static void handle_buttons_and_update()
{
    uint8_t b = btns_down;
    btns_down = a.buttonsState();
    btns_pressed = btns_down & ~b;
#ifdef REMOVE_TIMER0
    uint8_t updown = updown_frames;
    if((btns_down & (BTN_UP | BTN_DOWN)) == (BTN_UP | BTN_DOWN))
        ++updown;
    else
        updown = 0;
    if(updown == 45)
        a.exitToBootloader();
    updown_frames = updown;
#endif
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

static void delay5us()
{
    // this will be at least 80 cycles, which is 5us
    for(uint8_t i = 40; i != 0; --i)
        asm volatile("");
}

// Commands sent to the OLED display to initialize it
static const PROGMEM uint8_t lcdBootProgram[] = {
    0x20, 0x00,
    0x81, 0x00,
    0xDB, 0x20,
    0xD5, 0xF0,
    0x8D, 0x14,
    0xAF,
};

// avoids calls to delayShort
static void custom_bootOLED()
{
    // reset the display
    delay5us(); // reset pin should be low here. let it stay low a while
    bitSet(RST_PORT, RST_BIT); // set high to come out of reset
    delay5us(); // wait a while

    // select the display (permanently, since nothing else is using SPI)
    bitClear(CS_PORT, CS_BIT);

    // run our customized boot-up command sequence against the
    // OLED to initialize it properly for Arduboy
    
    abg_detail::send_cmds_prog<
        0x20, 0x00,
        0x81, 0x00,
        0xDB, 0x20,
        0xD5, 0xF0,
        0x8D, 0x14,
        0xAF
    >();
    
    
    //a.LCDCommandMode();
    //for (uint8_t i = 0; i < sizeof(lcdBootProgram); i++)
    //    a.SPItransfer(pgm_read_byte(lcdBootProgram + i));
    //a.LCDDataMode();
}

void setup()
{  
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

    a.bootPins();
    a.bootSPI();
    custom_bootOLED();
    a.bootPowerSaving();

#ifdef DEBUGBUILD
    Serial.begin(9600);
#endif
    
    FX::begin(FX_DATA_PAGE, 0);
    FX::enableOLED();
    initialize();
#ifndef DEBUG_MONOCHROME
    a.startGray();
#endif
}
