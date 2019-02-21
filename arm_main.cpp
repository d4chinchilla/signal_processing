#include "mbed.h"
#include <queue>
#include <vector>

#define ADC0_PIN D9
#define ADC1_PIN D10
#define ADC2_PIN D11
#define ADC3_PIN D12
#define CS_PIN D8
#define CLK_PIN D7

#define BUFFER_MAX 1024
#define DATA_BITS 13

DigitalOut cs(CS_PIN);
DigitalOut clk(D7);
BusIn adcs(D9, D10, D11, D12);
Serial serial(USBTX, USBRX);

//int sample_buffer[BUFFER_SIZE];
char serial_buffer[5]; // Includes 4 chars for sending samples, and a start packet
//int current_sample[4];
vector<int> current_sample;
queue< vector<int> > samples_buffer;

// Uses a bit-bashing method similar to SPI to read the values from all four ADCs. Then the
// values are manipulated into the correct format to be represented by a single int variable each.
// These can then be sent over serial.
// wait_us(1) is the smallest possible delay. If it turns out to be too large, this can be replaced
// with idle iterations through a for loop. This would be less exact and may need analysis on an
// oscilloscope to find frequencies.
void read_samples()
{
        int temp; // Stores ADC sample values, sample by sample, before they can be processed 
        
        // Bit-bashing the ADCs as if SPI
        // The data only starts on the third falling edge, so put two clocks outside the loop
        cs = 0;
        //wait_us(1);
        clk = 1;
        wait_us(1);
        clk = 0;
        wait_us(1);
        clk = 1;
        wait_us(1);
        clk = 0;
        
        for(uint8_t i = 0; i < DATA_BITS; i++)
        {
            clk = 1;
            wait_us(1);
            clk = 0;
            temp = adcs.read();
            
            current_sample[0] += (temp&1) << (DATA_BITS-(i+1));
            current_sample[1] += (temp&2) << (DATA_BITS-(i+2));
            current_sample[2] += (temp&4) << (DATA_BITS-(i+3));
            current_sample[3] += (temp&8) << (DATA_BITS-(i+4));
            
            // Debateable need for a delay in here to maintain clock pulse - hoping
            // the above logic is slow enough to generate a delay of about 1us anyway
        }
        
        // The MSB is a sign bit, and should always be 0. If it isn't, the bit
        // may have been corrupted and the sample should be set to 0.
        for(uint8_t i = 0; j < 4; i++)
        {
            if(current_sample[i] >> DATA_BITS-1)
                current_sample[i] = 0;
        }
        
        cs = 1;
}

int main()
{
    cs = 1;
    clk = 0;
    
    serial.baud(115200);    // THIS NEEDS TO BE SET THE SAME ON THE PI
    
    // Creates four elements in the vector to put samples in
    for(uint8_t i = 0; i < 4; i++)
    {
        current_sample.push_back(0);
    }
    
    for(;;)
    {
        read_samples();
        
        samples_buffer.push(current_sample);
        if(samples_buffer.size() == BUFFER_MAX)
            send_serial();
            
        // current_samples needs to be reset ready for the next iteration
        for(uint8_t i = 0; i < 4; i++)
        {
            current_sample[i] = 0;
        }
        

    }
    
    return 0;    
}