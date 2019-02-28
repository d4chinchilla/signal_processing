// Simple command line program to test the bit-shifting logic of the buffer
// code. Data on the ADC inputs is simulated using the a0-a3 arrays. Prints
// outputted char at the same time as it would send over serial.

#include <iostream>
using namespace std;

#define BUFFER_SIZE 1
#define DATA_BITS 9
#define NUM_MICS 4

uint8_t serial_buffer[NUM_MICS*BUFFER_SIZE];
uint16_t current_sample[NUM_MICS];
uint16_t samples_buffer[BUFFER_SIZE*NUM_MICS];
uint16_t top = 0;

// These test the important cases:
uint8_t a0[DATA_BITS] = {0,0,0,1,0,0,1,1,1};    // Standard number
uint8_t a1[DATA_BITS] = {1,1,1,0,1,1,0,1,0};    // Negative reading
uint8_t a2[DATA_BITS] = {0,1,1,1,1,1,1,1,1};    // 255 value
uint8_t a3[DATA_BITS] = {0,0,0,0,0,0,0,0,0};    // 0 value


void sample()
{
    for(uint8_t i = 0; i < DATA_BITS; i++)
    {
        current_sample[0] += a0[i] << (DATA_BITS-(i+1));
        current_sample[1] += a1[i] << (DATA_BITS-(i+1));
        current_sample[2] += a2[i] << (DATA_BITS-(i+1));
        current_sample[3] += a3[i] << (DATA_BITS-(i+1));
    }

    for(uint8_t i = 0; i < NUM_MICS; i++)
    {
        if(current_sample[i] >> (DATA_BITS-1))
            current_sample[i] = 0;
    }
}

void serial()
{
    for(uint16_t i = 0; i < BUFFER_SIZE*NUM_MICS; i++)
    {
        // cout << "samples_buffer: " << samples_buffer[i] << endl;
        serial_buffer[i] = (uint8_t)(samples_buffer[i]);

        if(serial_buffer[i] == 255)
            serial_buffer[i] = 254;

        // Have to cast serial_buffer[] else it tries to print like a char.
        // (Gives nonsense/unhelpful output)
        cout << "Sample " << i << ": " << (int)serial_buffer[i] << endl;

    }
    top = 0;
}

int main()
{
    sample();

    for(uint8_t i = 0; i < NUM_MICS; i++)
    {
        // cout << "top: " << top << endl;

        samples_buffer[top] = current_sample[i];
        top++;
    }

    if(top == BUFFER_SIZE*4)
        serial();

    return 0;
}