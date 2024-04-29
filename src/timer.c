#include "timer.h"
#include "spi.h"

extern uint8_t digit1, digit2;
extern uint8_t disp [];

extern uint8_t pattern1, pattern2;

void pb_debounce(void);

void timer_init(void) {
    cli();
//    TCB0.CTRLA = TCB_CLKSEL_DIV2_gc;    // Configure CLK_PER/2
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 33333;                   // Set interval for 10ms (33333 clocks @ 3.3 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable
    sei(); 
}

ISR(TCB0_INT_vect) {
    pb_debounce();
    static uint8_t digit = 0;    

    if (digit) {
        spi_write(disp[0] | (0x01 << 7));
    } else {
        spi_write(disp[1]);
    }
    digit = !digit;

    TCB0.INTFLAGS = TCB_CAPT_bm;
}

