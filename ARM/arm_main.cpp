// Code that runs on the Arm MCU buffer. Reads data from ADCs using SPI, then
// formats it into a serial packet and sends it to the back-end over serial.
// Maintains sampling rate of >50kHz.
// Written by Matthew Johns (mrj1g17@soton.ac.uk)
#include "mbed.h"

// Constants for pin numbers
#define ADC0_PIN A0
#define ADC1_PIN A1 
#define ADC2_PIN A2  
#define ADC3_PIN A3
#define CS_PIN D5
#define CLK_PIN D6

#define BUFFER_SIZE 1024
#define DATA_BITS 9
#define NUM_MICS 4
#define CLK_DELAY 8
#define BAUD 460800
#define START_BYTE 0xFF
DigitalOut cs(CS_PIN);
DigitalOut clk(CLK_PIN);
Serial serial(USBTX, USBRX);

char serial_buffer[NUM_MICS*BUFFER_SIZE];
uint16_t current_sample[NUM_MICS];
uint16_t samples_buffer[BUFFER_SIZE*NUM_MICS];
uint16_t top = 0;
uint8_t stall=0;

DigitalIn a0(ADC0_PIN);
DigitalIn a1(ADC1_PIN);
DigitalIn a2(ADC2_PIN);
DigitalIn a3(ADC3_PIN);

// Uses a bit-bashing method similar to SPI to read the values from all four 
// ADCs. Then the values are manipulated into the correct format to be 
// represented by a single int variable each.// These can then be sent over 
// serial.
void read_samples()
{   
    // Pulse clock once to get ADC sample going
    cs = 0;
    clk = 1;
    clk = 0;

    for(uint8_t i = 0; i < DATA_BITS; i++)
    {
        clk = 1;

    // Introduce delay to maintain square clock pulse and keep SPI clock below
    // 1MHz. #pragmas are to try and stop it being optimised out by compiler
#pragma GCC push_options
#pragma GCC optimze ("no-unroll-loops")
        for(uint8_t d=0; d <= CLK_DELAY; d++) {stall=d; __asm volatile("NOP");}
#pragma GCC pop_options

        clk = 0;

        current_sample[0] += a0 << (DATA_BITS-(i+1));
        current_sample[1] += a1 << (DATA_BITS-(i+1));
        current_sample[2] += a2 << (DATA_BITS-(i+1));
        current_sample[3] += a3 << (DATA_BITS-(i+1));
        
        // Above logic produces enough of a delay to not need an extra one
    }
    // The MSB is a sign bit, and should always be 0. If it isn't, the bit may
    // have been corrupted and the sample should be set to 0.
    for(uint8_t i = 0; i < NUM_MICS; i++)
    {
        if(current_sample[i] >> (DATA_BITS-1))
            current_sample[i] = 0;
    }
    
    cs = 1;
}

// Takes data in samples_buffer and makes it suitable for transmission, then 
// sends it
void send_serial()
{
    serial.putc(START_BYTE);
    for(uint16_t i = 0; i < BUFFER_SIZE*NUM_MICS; i++)
    {
        // Truncating sample to so the serial only sends the necessary 8 bits
        serial_buffer[i] = (uint8_t)(samples_buffer[i]);
        
        // Start byte is 0xff (255). If the sample == 255 it must be made 254
        // to avoid confusion. It's a small error so shouldn't cause issues
        if(serial_buffer[i] == 255)
            serial_buffer[i] = 254;

        // Sending the value of the sample
        serial.putc(serial_buffer[i]);
    }

    // Don't need to reset the samples buffer, as it will be overwritten. Just
    // say the top is the first element and new samples will be stored there
    top = 0;
}

int main()
{
    // Initial setup to keep the ADCs happy
    cs = 1;
    clk = 0;
    
    serial.baud(BAUD);
    
    // mbed OS scheduler thread suspected to be messing with timings. Make 
    // everything critical except serial tranmissions to ensure the sampling
    // rate is maintained
    CriticalSectionLock::enable();
    
    for(;;)
    {
        read_samples();
        
        // Adding the current sample to the buffer once it is retrieved
        for(uint8_t i = 0; i < NUM_MICS; i++)
        {
            samples_buffer[top] = current_sample[i];
            top++;
        }

        // Checks to see if the buffer is full. If so, sends serial.
        // Have to disable CriticalSectionLock, as serial uses interrupts which
        // cannot work when locked (crashes OS). Locked straight after though
        if(top == BUFFER_SIZE*4)
        {
            CriticalSectionLock::disable();
            send_serial();
            CriticalSectionLock::enable();
        }
    }
    
    return 0;    
}