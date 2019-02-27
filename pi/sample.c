#include "sample.h"
#include "sound.h"

int sample_packet_recv(packet_s *pkt, FILE *stream)
{
    int c, n;
    size_t micnum, samplenum;
    micnum    = 0;
    samplenum = 0;
    n = 0;

    // If there's clearly bullshit, run away
    while ((++n) < (100 * SAMPLE_SIZE))
    {
        c = fgetc(stream);

        if (c == -1)
        {
            printf("Error reading\n");
            return -1;
        }

        if (c == 0xff)
        {
            if (micnum == 0 && samplenum == 0)
                continue;
            else
            {
                printf("Unexpected 0xff\n");
                return -1;
            }
        }

        pkt->data[micnum][samplenum] = (int)c;
        micnum += 1;

        if (micnum == NUM_MICS)
        {
            micnum = 0;
            samplenum   += 1;
        }

        if (samplenum == SAMPLE_SIZE)
        {
            return 0;
        }
    }

    printf("No 0xff byte\n");
    return -1;
}

int sample_match_peaks(packet_s *pkt)
{
    sound_s sound;
    double peaks[NUM_XCORR][MAX_PEAKS];
    int    peakv[NUM_XCORR][MAX_PEAKS];
    int numpeaks[NUM_XCORR];
    int peak, xc;

    for (xc = 0; xc < NUM_XCORR; ++xc)
    {
        numpeaks[xc] = 0;
        peak = -1;
        while (numpeaks[xc] < MAX_PEAKS)
        {
            peak = xcorr_next_peak(pkt->xcorr[xc], peak);
            if (peak == -1)
                break;

            double dt = peak;
            dt -= XCORR_LEN / 2;
            dt /= SAMPLE_RATE;

            peaks[xc][numpeaks[xc]] = dt;
            peakv[xc][numpeaks[xc]] = pkt->xcorr[xc][peak];

            numpeaks[xc] += 1;
        }
    }

    sound_match_peaks(&sound,
        peaks[0], numpeaks[0], peakv[0],
        peaks[1], numpeaks[1], peakv[1],
        peaks[2], numpeaks[2], peakv[2]
    );
}
