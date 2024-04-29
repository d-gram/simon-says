//========== Include the required directories ===============================================================
#include <avr/io.h>
#include "timer.h"
#include "spi.h"
#include "buzzer.h"
#include "uart.h"
#include "sequencing.h"
#include "adc.h"

//========== Declare the variables ==========================================================================
volatile uint8_t pb_debounced = 0xFF; // debounced value of the button
volatile uint8_t pb_input = 0xFF; // input from the button
volatile uint8_t pb_state = 0xFF; // button state
static uint8_t vcount0=0, vcount1=0;   //vertical counter bits
uint8_t pb_old_state, pb_current_state = 0xFF; // Register previous PB state
uint8_t pb_new_state, pb_rising, pb_falling;
uint8_t sequence[200]; // Playback sequence array
int seq_values = 0; // Length of the playback sequence
uint8_t user_input[200]; // User input array
int user_input_index = 0; // Length of the user input array

const int display_digit_values[10] = {  // Values of the 7-segment display that correspond with decimal numbers
    0b00001000, //0
    0b01101011, //1
    0b01000100, //2
    0b01000001, //3
    0b00100011, //4
    0b00010001, //5
    0b00010000, //6
    0b01001000, //7
    0b00000000, //8
    0b00000011, //9
};

//========== Define the necessary functions ================================================================
void score_display(int user_input_index) {  // Display user's score
    if (user_input_index <= 99) {   // Check if the value is within 2-decimal range
        uint8_t tens = user_input_index / 10;   // Extract the tens
        uint8_t ones = user_input_index % 10;   // Extract the ones
        if (tens == 0) {    // if the score is below 10, keep the first digit off to distinguis between displaying, for example, 101 and 1.
            disp[0] = SEG_OFF;
            disp[1] = display_digit_values[ones];
        } else {
        disp[0] = display_digit_values[tens]; // Set the display values for each digit
        disp[1] = display_digit_values[ones];
        }
    } else {
        uint8_t ones = user_input_index % 10;   // If the score is greater than 99, display it with a leading zero as per requirments (ie display 08 for 108)
        disp[0] = display_digit_values[0];
        disp[1] = display_digit_values[ones];
    }
}

void pb_init(void) {    // Initialise pushbuttons
    // The buttons are configured as input py default
    // Enable pull-up resistors for full initialisation
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void pb_debounce(void) {    // Function for using the debounced button value
    pb_input = PORTA.IN; // Sample the button's input
    pb_debounced = (pb_input ^ pb_state);
    vcount1 = (vcount1 ^ vcount0) & pb_debounced;  // Vcount1 (bit 1 of vertical counter)
    vcount0 = ~vcount0 & pb_debounced;             // Vcount0 (bit 0 of vertical counter)

    pb_state ^= (vcount0 & vcount1);
}

typedef enum {  // Define the cases for main the state machine
    PLAYING,    // Playing the generated sequence
    WAIT,   // For the user's input
    CHECK,  // Check if the input matches the generated sequence
    TONE1,  // TONEs mainly handle buzzer and display behavior during button presses
    TONE2,
    TONE3,
    TONE4,
} state;


//========== Initialise the main loop ======================================================================
int main(void) {

    //========== Initialise the key functions ==============================================================
    pb_init();
    timer_init();
    spi_init();
    buzzer_init();
    uart_init();
    adc_init();

    //========== Declare states and variable of main parameters before starting the while loop ===
    state my_state = PLAYING;   // Starting case for the state switch case
    uint8_t button_pressed = 0;
    uint8_t correct;

    sequence_status = 1;    // sequence_status is a sequence-playing flag
    char valueStr[10];      // variable used for printing string through UART

    //========== Start the infinite while loop =============================================================
    while (1) {

        //========== Distinguish between rising and falling edge on buttons =========
        pb_old_state = pb_current_state;
        pb_current_state = pb_state;
        pb_new_state = pb_old_state ^ pb_current_state;

        // pressed means a falling edge from high -> low

        pb_falling = pb_new_state & pb_old_state;
        // released means a rising edge from low -> high

        pb_rising = pb_new_state & ~pb_old_state;

        
        //========== Enter the switch case flow control =============================================================
        switch (my_state) {
            case PLAYING:
                // uart_puts("case: playing\n");
                // uart_puts("Sequence: ");
                // for (int i = 0; i < seq_values; i++) {
                //     sprintf(valueStr, "%u ", sequence[i]); // Convert the value to a string
                //     uart_puts(valueStr); // Print the value
                // }
                // uart_puts("\n");
                // uart_puts("User Inputs: ");
                // for (int i = 0; i < user_input_index; i++) {
                //     sprintf(valueStr, "%u ", user_input[i]); // Convert the value to a string
                //     uart_puts(valueStr); // Print the value
                // }
                // uart_puts("\n");
                sequence[seq_values++] = sequence_gen();    // Generate the first value of the playback sequence
                while (sequence_status == 1) {  // If the playback is allowed, play the sequence
                    sequence_playback(sequence, seq_values);
                }

                user_input_index = 0;   // Reset the length of the user's input to 0
                my_state = WAIT;    // Switch to WAIT
            break;

            case WAIT:
                disp[0] = SEG_OFF;  // Make sure the display is off
                disp[1] = SEG_OFF;
                TCA0.SINGLE.PER = 0;    // Make sure the buzzer is off
                TCA0.SINGLE.CMP0 = 0;

                if (user_input_index != seq_values) {   // If the length of the user input array is shorter than the playback array, meaning the input is incomplete, check for buttons input
                    if (pb_falling & PIN4_bm) { // If S1 is pressed, go to the according TONE case
                        my_state = TONE1;
                        button_pressed = 1; // button_press flag, required to distinguish between button states
                    }
                    else if (pb_falling & PIN5_bm) {    // Same logic as S1
                        my_state = TONE2;
                        button_pressed = 1;
                    }
                    else if (pb_falling & PIN6_bm) {    // Same logic as S1
                        my_state = TONE3;
                        button_pressed = 1;
                    }
                    else if (pb_falling & PIN7_bm) {    // Same logic as S1
                        my_state = TONE4;
                        button_pressed = 1;
                    } else if (pb_rising & (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)) {   // If the a button is released, update the button_press flag
                        button_pressed = 0;
                    }
                    
                    if (!button_pressed) {  // If button_pressed flag = 0, meaning no button is pressed:
                        disp[0] = SEG_OFF;  // Turn off display
                        disp[1] = SEG_OFF;
                        TCA0.SINGLE.PER = 0;    // Turn off buzzer
                        TCA0.SINGLE.CMP0 = 0;
                        // this makes sure that the system is always in a defined state
                    }
                } else {    // If the length of the user input array is the same as the sequence playback array, switch to CHECK
                    my_state = CHECK;
                }

            break;

            case CHECK: 
                disp[0] = SEG_OFF;  // Make sure the display is off
                disp[1] = SEG_OFF;
                TCA0.SINGLE.PER = 0;    // Make sure the buzzer is off
                TCA0.SINGLE.CMP0 = 0;

                correct = 1;    // Correctness flag, used for judging the outcome of the comparison between input and generated sequence

                for (int i = 0; i < seq_values; i++) {  // Compare the values of the user input and generated array
                    if (user_input[i] != sequence[i]) { // If the values do not match (user made a mistake):
                        correct = 0;    // Set correctness flag to 0 and break the loop
                        break;
                    }
                }

                if (correct == 1) { // If the user's input was correct:
                    disp[0] = SEG_ON;   // display the SUCCESS message for the length of the playback delay
                    disp[1] = SEG_ON;
                    delay(1);

                    score_display(user_input_index);    // Display the score for the playback delay
                    delay(1);

                    my_state = PLAYING; // Return to PLAYING case to play the next sequence
                    sequence_status = 1;    // Set the sequence_status to 1 so that the next sequence is played when inside the PLAYING case

                } else if (correct == 0) {  // If the user's input was wrong:
                    disp[0] = SEG_G;    // Display the FAIl message for the playback delay
                    disp[1] = SEG_G;
                    delay(1);

                    score_display(user_input_index);    // Display the score for the playback delay
                    delay(1);

                    seq_values = 0; // Reset the length of playback sequence back to 0
                    user_input_index = 0;   // Reset the length of the user input aaray back to 0

                    my_state = PLAYING;
                    sequence_status = 1;
                }
            break;
            
            case TONE1:
                disp[0] = SEG_1;    // Light up the corresponding parts of the display
                disp[1] = SEG_OFF;

                TCA0.SINGLE.PER = TONE1_PER;    // Produce an apprapriate tone from the buzzer
                TCA0.SINGLE.CMP0 = (TONE1_PER / 2) + 1;

                delay(1);   // Do the above for the playback delay

                user_input[user_input_index++] = 0; // Add the value of the presed button to the user input array
                my_state = WAIT;    // Switch to WAIT to wait for further button presses
                break;

            case TONE2: // Same logic as TONE1
                disp[0] = SEG_2;
                disp[1] = SEG_OFF;

                TCA0.SINGLE.PER = TONE2_PER;
                TCA0.SINGLE.CMP0 = (TONE2_PER / 2) + 1;

                delay(1);

                user_input[user_input_index++] = 1;
                my_state = WAIT;
                break;

            case TONE3: // Same logic as TONE1
                disp[0] = SEG_OFF;
                disp[1] = SEG_1;

                TCA0.SINGLE.PER = TONE3_PER;
                TCA0.SINGLE.CMP0 = (TONE3_PER / 2) + 1;

                delay(1);

                user_input[user_input_index++] = 2;
                my_state = WAIT;
                break;

            case TONE4:  // Same logic as TONE1
                disp[0] = SEG_OFF;
                disp[1] = SEG_2;

                TCA0.SINGLE.PER = TONE4_PER;
                TCA0.SINGLE.CMP0 = (TONE4_PER / 2) + 1;

                delay(1);

                user_input[user_input_index++] = 3;
                my_state = WAIT;
                break;

            default:
                my_state = PLAYING;
        }
    }
}
