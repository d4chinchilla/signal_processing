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
Ticker clk_ctl;

char serial_buffer[NUM_MICS*BUFFER_SIZE];
uint16_t current_sample[NUM_MICS];
uint16_t samples_buffer[BUFFER_SIZE*NUM_MICS];
uint16_t top = 0;

const char start = 0x30;
uint8_t stall=0;
bool waiting = 1;
uint8_t bit = 0;

DigitalIn a0(D9);
DigitalIn a1(D10);
DigitalIn a2(D11);
DigitalIn a3(D12);

void sample_clk()
{
    clk = 1;
    // potentially need delay?
    clk = 0;

    current_sample[0] += a0 << (DATA_BITS-(bit+1));
    current_sample[1] += a1 << (DATA_BITS-(bit+1));
    current_sample[2] += a2 << (DATA_BITS-(bit+1));
    current_sample[3] += a3 << (DATA_BITS-(bit+1));

    waiting = 0;
}

void read_samples()
{
    // Pulse clock once to get ADC sample going
    cs = 0;
    clk = 1;
    clk = 0;

    // Begins the timer to trigger every 1us
    clk_ctl.attach_us(&sample_clk, 1);

    for(uint8_t i = 0; i < DATA_BITS; i++)
    {
        while(waiting) {}   // Idles to make sure sample is taken once every loop
        bit++;
        waiting = 1;
    }

    // Don't want clock to trigger any more, so switch off the ticker
    clk_ctl.detach();

    for(uint8_t i = 0; i < NUM_MICS; i++)
    {
        if(current_sample[i] >> (DATA_BITS-1))
            current_sample[i] = 0;
    }

    cs = 1;
    bit = 0;
}

void send_serial()
{
    // START_BYTE precedes every transmission to tell backend where it begins
    serial.putc(START_BYTE);

    // Iterate through entire buffer and send each value
    for(uint16_t i = 0; i < BUFFER_SIZE*NUM_MICS; i++)
    {
        serial_buffer[i] = (uint8_t)(samples_buffer[i]);

        // The start byte is 0xff (255) so if the sample == 255 it must be made 254
        // to avoid confusion. It's a small error so won't cause issues
        if(serial_buffer[i] == 255)
            serial_buffer[i] = 254;

        serial.putc(serial_buffer[i]);
    }

    // No need to empty buffer each time, as it will just be overwritten anyway. Just
    // tell it to start at the beginning of the buffer again
    top = 0;
}

int main()
{
    cs = 1;
    clk = 0;

    serial.baud(BAUD);

    for(;;)
    {
        CriticalSectionLock::enable();

        read_samples();

        // In each iteration, transfers contents of recent sample into buffer
        for(uint8_t i = 0; i < NUM_MICS; i++)
        {
            samples_buffer[top] = current_sample[i];
            top++;
        }

        // Detects if the buffer is full, and if so, sends it all
        if(top == BUFFER_SIZE*4)
        {
            CriticalSectionLock::disable();
            send_serial();
        }

        // // current_samples needs to be reset ready for the next iteration
        // for(uint8_t i = 0; i < NUM_MICS; i++)
        // {
        //     current_sample[i] = 0;
        // }
    }
}