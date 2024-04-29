
#include "spi.h"

// SPI configuration and spi_write functionality (CAB202, 2023)
void spi_init(void) {
    cli();
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc;   // SPI pins on PC0-3
    PORTC.DIRSET = (PIN0_bm | PIN2_bm);         // Set SCK (PC0) and MOSI (PC2) as outputs
    PORTA.OUTSET = PIN1_bm;                     // DISP LATCH as output, initially high
    PORTA.DIRSET = PIN1_bm;    
    
    SPI0.CTRLA = SPI_MASTER_bm;                 // Master, /4 prescaler, MSB first  
    SPI0.CTRLB = SPI_SSD_bm;                    // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;                   // Interrupt enable    
    SPI0.CTRLA |= SPI_ENABLE_bm;                // Enable
    sei();
}

void spi_write(uint8_t b) {
    SPI0.DATA = b;
}

ISR(SPI0_INT_vect) {
    // Rising edge DISP LATCH
    PORTA.OUTCLR = PIN1_bm; 
    PORTA.OUTSET = PIN1_bm;
    SPI0.INTFLAGS = SPI_IF_bm;
}
