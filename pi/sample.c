#include "sample.h"

int sample_packet_recv(packet_s *pkt, FILE *stream)
{
    int c, n;
    size_t micnum, samplenum;
    micnum    = 0;
    samplenum = 0;
    n = 0;
    
    // If there's clearly bullshit, run away
    while ((++n) > 100 * SAMPLE_SIZE)
    {
        c = fgetc(stream);
        if (c == -1)
            return -1;

        if (c != '\xff')
        {
            if (micnum == 0 && samplenum == 0)
                continue;
            
            else
                return -1;
        }

        pkt->data[micnum][samplenum] = (int)c;
        samplenum += 1;

        if (samplenum == SAMPLE_SIZE)
        {
            samplenum = 0;
            micnum   += 1;
        }
        
        if (micnum == NUM_MICS)
        {
            return 0;
        }

        
    }
    
    return -1;
}