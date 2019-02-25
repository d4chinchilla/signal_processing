#include "mbed.h"
#include <queue>
#include <vector>

#define ADC0_PIN D9
#define ADC1_PIN D10
#define ADC2_PIN D11
#define ADC3_PIN D12
#define CS_PIN D8
#define CLK_PIN D7

#define BUFFER_SIZE 2048   // If this goes above 255 then some variables may need to be expanded
#define DATA_BITS 9
#define NUM_MICS 4
#define CLK_DELAY 3
#define BAUD 1500000
#define START_BYTE 0XFF
DigitalOut cs(CS_PIN);
DigitalOut clk(D7);
//BusIn adcs(D9, D10, D11, D12);
RawSerial serial(USBTX, USBRX);

//int sample_buffer[BUFFER_SIZE];
//char serial_buffer[ 1+ 7*BUFFER_SIZE ]; // Includes 4 chars for sending samples, and a start packet
char serial_buffer[NUM_MICS*BUFFER_SIZE];
//int current_sample[4];
//vector<int> current_sample;
uint16_t current_sample[NUM_MICS];
uint16_t samples_buffer[BUFFER_SIZE*NUM_MICS];
//queue< vector<int> > samples_buffer;
uint16_t top = 0;
const char start = 0x30;

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
        //wait_us(1);
        clk = 1;
//        wait_us(1);
        for(uint8_t d=0; d <= CLK_DELAY; d++) {__nop();}
        clk = 0;
//        wait_us(1);
        for(uint8_t d=0; d <= CLK_DELAY; d++) {__nop();}
        clk = 1;
//        wait_us(1);
        for(uint8_t d=0; d <= CLK_DELAY; d++) {__nop();}
        clk = 0;
        
        for(uint8_t i = 0; i < DATA_BITS; i++)
        {
            clk = 1;
//            wait_us(1);
            for(uint8_t d=0; d <= CLK_DELAY-1; d++) {__nop();}
            clk = 0;
//            temp = adcs.read();
            //a0.read();
////            a1.read();
////            a2.read();
////            a3.read();
            
            current_sample[0] += a0 << (DATA_BITS-(i+1));
            current_sample[1] += a1 << (DATA_BITS-(i+1));
            current_sample[2] += a2 << (DATA_BITS-(i+1));
            current_sample[3] += a3 << (DATA_BITS-(i+1));
            
            // Debateable need for a delay in here to maintain clock pulse - hoping
            // the above logic is slow enough to generate a delay of about 1us anyway
        }
        
        // The MSB is a sign bit, and should always be 0. If it isn't, the bit
        // may have been corrupted and the sample should be set to 0.
        for(uint8_t i = 0; i < NUM_MICS; i++)
        {
            if(current_sample[i] >> DATA_BITS-1)
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

//    serial.putc(START_BYTE);
    serial.putc(0x30);
    for(uint16_t i = 0; i < BUFFER_SIZE*NUM_MICS; i++)
    {
//        serial.printf("serial loop\n");
        serial_buffer[i] = (uint8_t)(samples_buffer[i] >> 4);
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