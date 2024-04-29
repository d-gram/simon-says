#include "uart.h"

void uart_init(void) {
    cli();
    PORTB.DIRSET = PIN2_bm; // Enable PB2 as output (USART0 TXD)
    USART0.BAUD = 1389;     // 9600 baud @ 3.3 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;   // Enable Tx/Rx
    sei();
}

uint8_t uart_getc(void) {
    while (!(USART0.STATUS & USART_RXCIF_bm));  // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c) {
    while (!(USART0.STATUS & USART_DREIF_bm));  // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

void uart_puts(char* string) {
    char *ptr = string;  // Create a pointer to the input string 
    while (*ptr != '\0') {  // Iterate through the string items and print
        uart_putc(*ptr);    // them one by one until reaching the end
        ptr++;
    }
}