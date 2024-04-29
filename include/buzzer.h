#include <avr/io.h>
#include <avr/interrupt.h>

 /***
  * Determine your frequencies and update the #defines below to reflect these values.
  */
#define TONE1_PER 9662
#define TONE2_PER 11494
#define TONE3_PER 7231
#define TONE4_PER 19268

void buzzer_init(void);
