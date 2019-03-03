#include <string.h>
#include "xcorr.h"
#include "sample.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static void  xcorr_job_init(xcorr_job_s *job, int *a, int *b, int *res);
static void  xcorr_job_kill(xcorr_job_s *job);
static void *xcorr_job_main(void *arg);

static void *xcorr_manager_main(void *arg);

static void xcorr_job_init(xcorr_job_s *job, int *a, int *b, int *res)
{
    job->a   = a;
    job->b   = b;
    job->res = res;

    pthread_create(&(job->thread), NULL, xcorr_job_main, job);
}

static void xcorr_job_kill(xcorr_job_s *job)
{
    pthread_join(job->thread, NULL);
}

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)


static void xcorr_norm(int *a)
{
    int ind;
    double avg;
    avg = 0.0;
    for (ind = 0; ind < SAMPLE_SIZE; ++ind)
    {
        avg += a[ind];
    }

    avg /= SAMPLE_SIZE;

    for (ind = 0; ind < SAMPLE_SIZE; ++ind)
    {
        a[ind] -= avg;
    }
}

static void xcorr(int *a, int *b, int *res)
{
    int offset, offind, ind;

    for (offind = 0; offind < XCORR_LEN; ++offind)
    {
        int sum;
        sum    = 0;
        offset = offind - (XCORR_LEN / 2);

        for (ind = offset; ind < SAMPLE_SIZE - XCORR_LEN + offset; ++ind)
        {
            sum += a[ind] * b[ind + offset];
        }
        res[offind] = sum;
    }
}

static void *xcorr_job_main(void *arg)
{
    xcorr_job_s *job;

    job = arg;
    xcorr(job->a, job->b, job->res);

    return NULL;
}

void xcorr_manager_init(xcorr_manager_s *job)
{
    job->running = 1;
    job->packet = malloc(sizeof(packet_s));

    pthread_create(&(job->thread), NULL, xcorr_manager_main, job);
}

void xcorr_manager_kill(xcorr_manager_s *job)
{
    job->running = 0;

    pthread_join(job->thread, NULL);
}

static void *xcorr_manager_main(void *arg)
{
    packet_s *pkt;
    FILE *f;
    xcorr_manager_s *job;
    xcorr_job_s     *workers;

    job     = arg;
    workers = job->workers;
    pkt     = job->packet;
    job->calibratingstarted = 0;

    memset(job->calib, 0, sizeof(job->calib));
    job->ncalib = 1;

    // When this is working fully, we don't need to mkfifo!
    mkfifo("/tmp/chinchilla-serial", 0666);
    f = fopen("/tmp/chinchilla-serial", "r");

    while (job->running)
    {
        int njob, ind;

        if (sample_packet_recv(pkt, f) != 0)
        {
            usleep(100000);
            continue;
        }

        for (ind = 0; ind < NUM_MICS; ++ind)
            xcorr_norm(pkt->data[ind]);

        for (njob = 0; njob < NUM_XCORR; ++njob)
            xcorr_job_init(
                &(workers[njob]),
                pkt->data[0], pkt->data[1 + njob], pkt->xcorr[njob]
            );

        for (njob = 0; njob < NUM_XCORR; ++njob)
            xcorr_job_kill(&(workers[njob]));

        // This is the case where calibrating has just been started.
        if (job->calibrating && !job->calibratingstarted)
        {
            puts("Starting calibration");
            job->calibratingstarted = 1;
            memset(job->calib, 0, sizeof(job->calib));
            // This the ongoing case is run
        }

        // This is the case where calibrating is ongoing
        if (job->calibrating && job->calibratingstarted)
        {
            int xc;
            for (xc = 0; xc < NUM_XCORR; ++xc)
            {
                int ind;
                for (ind = 0; ind < XCORR_LEN; ++ind)
                {
                    job->calib[xc][ind] += pkt->xcorr[xc][ind];
                }
            }
            job->ncalib += 1;
        }
        // This is the case where calibrating has just stopped
        else if (!job->calibrating && job->calibratingstarted)
        {
            if (job->ncalib)
            {
                puts("Ending calibration");
                int xc;
                for (xc = 0; xc < NUM_XCORR; ++xc)
                {
                    int ind;
                    for (ind = 0; ind < XCORR_LEN; ++ind)
                    {
                        job->calib[xc][ind] /= job->ncalib;
                    }
                }
            }
            else
            {
                puts("Empty calibration :( I was told to calibrate but got no data");
            }
            job->calibratingstarted = 0;
            // Then the normal case is run
        }

        // This is the normal case
        if (!job->calibrating)
        {
            int xc;
            for (xc = 0; xc < NUM_XCORR; ++xc)
            {
                int ind;
                for (ind = 0; ind < XCORR_LEN; ++ind)
                {
                    pkt->xcorr[xc][ind] -= job->calib[xc][ind];
                }
            }
            sample_match_peaks(pkt);
        }
        int xc;
        for (xc = 0; xc < 3; ++xc)
        {
            ind = -1;
            while ((ind = xcorr_next_peak(pkt->xcorr[xc], ind)) != -1)
            {
                printf("%d: %d\n", xc, ind);
            }
        }

    }

    return NULL;
}

int xcorr_next_peak(int *vals, int prev)
{
    int peak, off;
    if (prev != -1)
        peak = prev + PEAK_X_THRESHOLD;
    else
        peak = PEAK_X_THRESHOLD;

    while (peak < XCORR_LEN - PEAK_X_THRESHOLD)
    {
        // Iterate forward and see if the current peak is
        //  a maximum forwards
        for (off = 0; off < PEAK_X_THRESHOLD; ++off)
        {
            if (vals[peak] < vals[peak + off])
                break;
        }

        // If there is no larger peak forwards
        if (off == PEAK_X_THRESHOLD)
        {
            // Iterate backwards and see if the peak is the maximum
            //  looking backwards
            for (off = 0; off > -PEAK_X_THRESHOLD; --off)
            {
                if (vals[peak] < vals[peak + off])
                    break;
            }

            // If it is, and it is over the Y threshold, it is a peak
            if (off == -PEAK_X_THRESHOLD && vals[peak] > PEAK_Y_THRESHOLD)
                return peak;
            else
                peak += PEAK_X_THRESHOLD;
        }
        else
        {
            peak += off;
        }
    }
    return -1;
}
