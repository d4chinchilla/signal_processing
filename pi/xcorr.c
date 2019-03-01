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

        for (ind = MAX(0, -offset); ind < MIN(SAMPLE_SIZE, SAMPLE_SIZE - offset); ++ind)
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

        for (ind = 0; ind < SAMPLE_SIZE; ++ind)
        {
          //  printf("%d %d %d %d\n", pkt->data[0][ind],pkt->data[1][ind],pkt->data[2][ind], pkt->data[3][ind]);
        }

        for (ind = 0; ind < XCORR_LEN; ++ind)
        {
         //   printf("%d: %d %d %d\n", ind, pkt->xcorr[0][ind],pkt->xcorr[1][ind],pkt->xcorr[2][ind]);
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

        sample_match_peaks(pkt);
    }
    
    fclose(f);

    return NULL;
}

#define PEAK_X_THRESHOLD 10

int xcorr_next_peak(int *vals, int prev)
{
    int peak, off;
    if (prev != -1)
        peak = prev + PEAK_X_THRESHOLD;
    else
        peak = PEAK_X_THRESHOLD;

    while (peak < XCORR_LEN - PEAK_X_THRESHOLD)
    {
        for (off = 0; off < PEAK_X_THRESHOLD; ++off)
        {
            if (vals[peak] < vals[peak + off])
                break;
        }

        if (off == PEAK_X_THRESHOLD)
        {
            for (off = 0; off > -PEAK_X_THRESHOLD; --off)
            {
                if (vals[peak] < vals[peak + off])
                    break;
            }

            if (off == -PEAK_X_THRESHOLD)
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
