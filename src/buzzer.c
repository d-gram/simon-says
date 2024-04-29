#include "buzzer.h"

void buzzer_init(void) {    // Initialise the buzzer
    cli();
    PORTB.OUTSET = PIN0_bm;        // Turn the buzzer off
    PORTB.DIRSET = PIN0_bm;

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;  
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;   // SINGLE SLOPE, W00
    
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // enable peripheral
    sei();
}