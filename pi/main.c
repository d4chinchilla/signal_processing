#include "sample.h"

void main(void)
{
    int mic, ind;
    packet_s pkt;
    if (sample_packet_recv(&pkt, stdin))
    {
        puts(":(");
        return;
    }
    for (mic = 0; mic < NUM_MICS; ++mic)
    {
        for (ind = 0; ind < SAMPLE_SIZE; ++ind)
        {
            printf("%d, ", pkt.data[mic][ind]);
        }
        puts("");
    }
}