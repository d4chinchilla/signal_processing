#include "sample.h"
#include "sound.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* Use select to wait until a stream is readable.  *
 * There is a 1 second timeout on this function.   *
 * It returns 1 if the stream has become readable, *
 * and 0 otherwise.                                */
int wait_for_file(FILE *stream)
{
    int fn;
    /* Timeout */
    struct timeval tout = { .tv_sec = 0, .tv_usec = 1000000 };
    fd_set waitfor;
    fn = fileno(stream);

    /* Set the appropriate bits in the fd_set */
    FD_ZERO(&waitfor);
    FD_SET(fn, &waitfor);

    /* Wait for the fd */
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

        /* Get the next character */
        c = fgetc(stream);

        if (feof(stream))
        {
            /* Clear EOF or we'll continually read EOF chars */
            clearerr(stream);
            return -1;
        }
        else if (c == EOF)
        {
            /* Other errors */
            clearerr(stream);
            printf("Error reading: %s\n", strerror(errno));
            return -1;
        }

        /* The starting character */
        if (c == 0xff)
        {
            /* It is expected as the first character */
            if (micnum == 0 && samplenum == 0)
                continue;
            /* But not in other positions */
            else
            {
                printf("Unexpected 0xff\n");
                return -1;
            }
        }

        /* Read the data point */
        pkt->data[micnum][samplenum] = (int)c;
        micnum += 1;

        /* If we're done with a group of four mic readings, *
         * increment the sample position.                   */
        if (micnum == NUM_MICS)
        {
            micnum = 0;
            samplenum   += 1;
        }

        /* If we're out of samples to read, we're done with *
         * a packet!                                        */
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
    /* Vectors to store peaks and their amplitudes in */
    double peaks[NUM_XCORR][MAX_PEAKS]; /* They are stored as times in seconds here */
    int    peakv[NUM_XCORR][MAX_PEAKS]; /* Amplitudes are stored here               */
    int numpeaks[NUM_XCORR];
    int peak, xc;

    /* For each cross correlation */
    for (xc = 0; xc < NUM_XCORR; ++xc)
    {
        numpeaks[xc] = 0;
        peak = -1;
        while (numpeaks[xc] < MAX_PEAKS)
        {
            /* Get the next peak */
            peak = xcorr_next_peak(pkt->xcorr[xc], peak);
            if (peak == -1)
                break;

            /* Convert the peak offset to a time delta */
            double dt = peak;
            dt -= XCORR_LEN / 2;
            dt /= SAMPLE_RATE;

            /* Set the peak position and value */
            peaks[xc][numpeaks[xc]] = dt;
            peakv[xc][numpeaks[xc]] = pkt->xcorr[xc][peak];

            numpeaks[xc] += 1;
        }
    }

    /* Match the sets of peaks to sounds */
    sound_match_peaks(&sound,
        peaks[0], numpeaks[0], peakv[0],
        peaks[1], numpeaks[1], peakv[1],
        peaks[2], numpeaks[2], peakv[2]
    );
}
