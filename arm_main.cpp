#include "mbed.h"

#define ADC0_PIN PA_0
#define ADC1_PIN PA_1
#define ADC2_PIN PA_3
#define ADC3_PIN PA_4
#define CS_PIN D5
#define CLK_PIN D6

#define BUFFER_SIZE 10
#define DATA_BITS 9
#define NUM_MICS 4
#define CLK_DELAY 8
#define BAUD 1500000
#define START_BYTE 0xFF
DigitalOut cs(CS_PIN);
DigitalOut clk(CLK_PIN);
Serial serial(USBTX, USBRX);

char serial_buffer[NUM_MICS*BUFFER_SIZE];
uint16_t current_sample[NUM_MICS];
uint16_t samples_buffer[BUFFER_SIZE*NUM_MICS];
uint16_t top = 0;
const char start = 0x30;
uint8_t stall=0;

DigitalIn a0(D9);
DigitalIn a1(D10);
DigitalIn a2(D11);
DigitalIn a3(D12);

// Uses a bit-bashing method similar to SPI to read the values from all four ADCs. Then the
// values are manipulated into the correct format to be represented by a single int variable each.
// These can then be sent over serial.
// wait_us(1) is the smallest possible delay. If it turns out to be too large, this can be replaced
// with idle iterations through a for loop. This would be less exact and may need analysis on an
// oscilloscope to find frequencies.
void read_samples()
{        
        // Bit-bashing the ADCs as if SPI
        // The data only starts on the third falling edge, so put two clocks outside the loop
        cs = 0;
        // wait_us(1);
//         for(uint8_t d=0; d <= CLK_DELAY; d++) { __asm volatile("NOP"); }
        clk = 1;
// // #pragma GCC push_options
// // #pragma GCC optimze ("no-unroll-loops")
        // for(uint8_t d=0; d <= CLK_DELAY; d++) { __asm volatile("NOP"); }
//         // wait_us(1);
        clk = 0;
// #pragma GCC pop_options
        for(uint8_t i = 0; i < DATA_BITS; i++)
        {
            clk = 1;
#pragma GCC push_options
#pragma GCC optimze ("no-unroll-loops")
            for(uint8_t d=0; d <= CLK_DELAY; d++) { stall=d; __asm volatile("NOP"); }
            // wait_us(1);
#pragma GCC pop_options
            clk = 0;

            current_sample[0] += a0 << (DATA_BITS-(i+1));
            current_sample[1] += a1 << (DATA_BITS-(i+1));
            current_sample[2] += a2 << (DATA_BITS-(i+1));
            current_sample[3] += a3 << (DATA_BITS-(i+1));
            
            // wait_us(1);
            // Debateable need for a delay in here to maintain clock pulse - hoping
            // the above logic is slow enough to generate a delay of about 1us anyway
        }
        // The MSB is a sign bit, and should always be 0. If it isn't, the bit
        // may have been corrupted and the sample should be set to 0.
        for(uint8_t i = 0; i < NUM_MICS; i++)
        {
            if(current_sample[i] >> (DATA_BITS-1))
                current_sample[i] = 0;
        }
        
        cs = 1;
}

void send_serial()
{
//    serial.printf("Trying to print buffer\n");
//    serial_buffer[0] = 0x30;
//    serial_buffer[1] = 0x34;
//    serial.printf("Sending serial\n");

    // serial.putc(START_BYTE);
    while(!serial.writeable()){}
    serial.putc(0x30);
    for(uint16_t i = 0; i < BUFFER_SIZE*NUM_MICS; i++)
    {
//        serial.printf("serial loop\n");
        serial_buffer[i] = (uint8_t)(samples_buffer[i] >> 4);
        // while(!serial.writeable()){}
        serial.putc(serial_buffer[i]);
    }
//    serial.printf(&start);
//    serial.printf(serial_buffer);
    top = 0;
}

int main()
{
    cs = 1;
    clk = 0;
    
    serial.baud(BAUD);    // THIS NEEDS TO BE SET THE SAME ON THE PI
    // serial.set_blocking(true);
    
//    serial.printf("Started\n");
    
    for(;;)
    {
//        serial.printf("Loop\n");
        read_samples();
        
        for(uint8_t i = 0; i < NUM_MICS; i++)
        {
//            serial.printf("Storing to buffer\n");
//            serial.printf("%d\n", top);
            samples_buffer[top] = current_sample[i];
            top++;
        }
//        top += 4;
//        serial.printf("Checking %d = %d\n", top, (BUFFER_SIZE*4));
        if(top == BUFFER_SIZE*4)
            send_serial();
            
        // current_samples needs to be reset ready for the next iteration
        for(uint8_t i = 0; i < NUM_MICS; i++)
        {
            current_sample[i] = 0;
        }

    }
    
    return 0;    
}