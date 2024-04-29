#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#define SEG_1  0b00111110   // SEG_1, SEG_2 and SEG_OFF represent the binary
#define SEG_2  0b01101011   // values of the 7-segment display, corresponding to the
#define SEG_OFF 0b11111111  // "vertical lines" (SEG_1, SEG_2), and display off (SEG_OFF)
#define SEG_ON 0b00000000
#define SEG_G 0b01110111
#define TONE1_PER 9662
#define TONE2_PER 11494
#define TONE3_PER 7231
#define TONE4_PER 19268

extern uint8_t potentiometer;
extern uint8_t disp[];  //disp initially off
extern uint32_t state_lfsr;
extern uint8_t step;
extern volatile int sequence_status;
extern uint32_t clock_cycles;
extern float scaled_reading;

uint8_t sequence_gen(void);
void play(uint8_t value);
void sequence_playback(uint8_t values[], int numValues);
void delay(int percent);
