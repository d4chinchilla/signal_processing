#include "sample.h"
#include "sound.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

int wait_for_file(FILE *stream)
{
    int fn;
    struct timeval tout = { .tv_sec = 0, .tv_usec = 100000 };
    fd_set waitfor;
    fn = fileno(stream);

    FD_ZERO(&waitfor);
    FD_SET(fn, &waitfor);
    
    if (select(fn + 1, &waitfor, NULL, NULL, &tout) == 1)
    {
        return 1;
    }
    
    return 0;
}

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
        // This is an experimental optimization, kill it if you want <3 - francis
        if (!wait_for_file(stream))
        {
            puts("Timed out waiting for input");
            return -1;
        }

        c = fgetc(stream);

        if (feof(stream))
        {
            clearerr(stream);
            return -1;
        }
        else if (c == EOF)
        {
            printf("Error reading: %s\n", strerror(errno));
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
            puts("Got sample");
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
