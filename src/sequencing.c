//========== Include the required directories ===============================================================
#include "sequencing.h"
#include "uart.h"

//========== Declare the variables ==========================================================================
uint32_t state_lfsr = 0x11791269;   // Seed (was given) for the pseudo-random number generation
uint8_t step;   // Varibale to hold the pseudo-random generated number
uint8_t disp[] = {SEG_OFF, SEG_OFF};  // Disp initially off
volatile int sequence_status;   // Sequence playing status (0 or 1)
uint8_t potentiometer;  // Variable for the potentiometer input
float scaled_reading;   // Variables used for converting between potentiometer reading (0 - 255) and a delay (0.25 - 2 seconds)
uint32_t clock_cycles;

uint8_t sequence_gen(void) {    // Linear-feedback shift register function
    uint32_t *state_ptr = &state_lfsr; // initial value of seed variable
    uint32_t shifted = *state_ptr >> 1; // Right shift the bits in "state_lfsr" by 1 bit position.
    if (*state_ptr & 1) { // If the bit shifted out of state was set (1)
        shifted ^= 0xE2023CAB; // XOR of "state" with MASK 0xE2023CAB
    }
    *state_ptr = shifted; // update state
    // STEP ← STATE_LFSR and 0b11
    return step = shifted & 0b11;
}

void delay(int percent) {   // Delay generator function
    potentiometer = ADC0.RESULT0;   // Read the potentiometer input
    scaled_reading = (((float)potentiometer / 255.0) * (2 - 0.25)) + 0.25;  // Re-scale the reading from potentiometer (0-255 range) to required delay range in seconds (0.25 to 2)
    clock_cycles = (scaled_reading * 66666)/percent;  // Calculate the amount of clock cycles required to fulfill the desired delay duration
    for (volatile uint32_t i = 0; i < clock_cycles; i++) {  // Create a delay loop
        // Empty loop for the delay
    }
}

void play(volatile uint8_t value) {     // Sequence playing function
    // Value in sequence range from 0 to 3.
    // Each value has a corresponding tone and segment fo display
    // The switch cases handle the appropriate sequence playback, based on the read values of sequence
    if (value == 0) { 
            disp[0] = SEG_1;
            disp[1] = SEG_OFF;
            TCA0.SINGLE.PER = TONE1_PER;
            TCA0.SINGLE.CMP0 = (TONE1_PER / 2) + 1;
    } else if (value == 1) {
            disp[0] = SEG_2;
            disp[1] = SEG_OFF;
            TCA0.SINGLE.PER = TONE2_PER;
            TCA0.SINGLE.CMP0 = (TONE2_PER / 2) + 1;
    } else if (value == 2) {
            disp[0] = SEG_OFF;
            disp[1] = SEG_1;
            TCA0.SINGLE.PER = TONE3_PER;
            TCA0.SINGLE.CMP0 = (TONE3_PER / 2) + 1;
    } else if (value == 3) {
            disp[0] = SEG_OFF;
            disp[1] = SEG_2;
            TCA0.SINGLE.PER = TONE4_PER;
            TCA0.SINGLE.CMP0 = (TONE4_PER / 2) + 1;
    }
}

void sequence_playback(uint8_t values[], int seq_values) {  // Main sequence playback function
    for (uint8_t i = 0; i < seq_values && sequence_status == 1; i++) {  // If the sequence_status flag is set, play the sequence's elements one by one until reaching the end of the sequence
        play(values[i]);
        delay(2);   // Each step of the sequence is played for half of the playback delay
    }
    sequence_status = 0;    // Aknowledge the serviced sequence playback by clearing the flag
}
