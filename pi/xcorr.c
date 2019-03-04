#include <string.h>
#include "fft/wrap.h"
#include "xcorr.h"
#include "sample.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

/* This is the file where our program spends most of its time *
 * it is where all the management of threads takes place      */

/* The threading model is simple, three child threads are launched  *
 * by one manager thread, and associated with a pair of microphones *
 * to crosscorrelate. Condition locks are used by all the threads   *
 * to wait for their manager to signal a go, and then are used by   *
 * the manager to wait for each thread to finish. The manager       *
 * meanwhile does an FFT of the data.                               */
static void  xcorr_job_init(xcorr_job_s *job, int *a, int *b, int *res);
static void  xcorr_job_kill(xcorr_job_s *job);
static void *xcorr_job_main(void *arg);
static void xcorr_job_wait(xcorr_job_s *job);

static void *xcorr_manager_main(void *arg);

/* Wait for a job to finish */
static void xcorr_job_wait(xcorr_job_s *job)
{
    pthread_cond_wait(&(job->done), &(job->done_mtx));
}

/* Initialize a job */
static void xcorr_job_init(xcorr_job_s *job, int *a, int *b, int *res)
{
    job->a   = a;
    job->b   = b;
    job->res = res;
    job->running = 1;

    /* Start the pair of condition locks */
    pthread_mutex_init(&(job->launch_mtx), NULL);
    pthread_cond_init(&(job->launch), NULL);
    pthread_mutex_init(&(job->done_mtx), NULL);
    pthread_cond_init(&(job->done), NULL);

    /* Lock the done mutex before setting the thread, that way *
     * we can wait for it to send a done condition when it is  *
     * initialized.                                            */
    pthread_mutex_lock(&(job->done_mtx));

    pthread_create(&(job->thread), NULL, xcorr_job_main, job);

    xcorr_job_wait(job);
}

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

/* Normalize a sample to be zero average */
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

/* Do a cross-correlation */
static void xcorr(int *a, int *b, int *res)
{
    int offset, offind, ind;

    /* To keep the cross correlation flat, we do each offset with the *
     * same number of samples.                                        */
    for (offind = 0; offind < XCORR_LEN; ++offind)
    {
        int sum;
        sum    = 0;
        offset = offind - (XCORR_LEN / 2);

        for (ind = (XCORR_LEN / 2) - offset; ind < SAMPLE_SIZE - (XCORR_LEN / 2) - offset; ++ind)
        {
            sum += a[ind] * b[ind + offset];
        }
        res[offind] = sum;
    }
}

/* Launch a job */
static void xcorr_job_launch(xcorr_job_s *job)
{
    pthread_mutex_lock(&(job->launch_mtx));
    pthread_cond_signal(&(job->launch));
    pthread_mutex_unlock(&(job->launch_mtx));
}

/* Kill a job */
static void xcorr_job_kill(xcorr_job_s *job)
{
    job->running = 0;
    /* It must be launched first, so that it isn't blocked on its condition */
    xcorr_job_launch(job);
    pthread_join(job->thread, NULL);
}

static void *xcorr_job_main(void *arg)
{
    xcorr_job_s *job;
    job = arg;

    /* Start by locking the launch mutex */
    pthread_mutex_lock(&(job->launch_mtx));

    /* Signal we are initialized. We must do this after locking launch, *
     * to avoid race conditions!                                        */
    pthread_mutex_lock(&(job->done_mtx));
    pthread_cond_signal(&(job->done));
    pthread_mutex_unlock(&(job->done_mtx));

    while (job->running)
    {
        /* Wait to be launched */
        pthread_cond_wait(&(job->launch), &(job->launch_mtx));

        /* If we're no longer alive, die */
        if (!job->running)
            break;

        /* Do our job */
        xcorr(job->a, job->b, job->res);

        /* Signal that we are done now! */
        pthread_mutex_lock(&(job->done_mtx));
        pthread_cond_signal(&(job->done));
        pthread_mutex_unlock(&(job->done_mtx));
    }
    puts("DONE");
    return NULL;
}

/* Initialize the manager thread */
void xcorr_manager_init(xcorr_manager_s *job)
{
    job->running = 1;
    job->packet = malloc(sizeof(packet_s));

    pthread_create(&(job->thread), NULL, xcorr_manager_main, job);
}

/* Kill the manager thread */
void xcorr_manager_kill(xcorr_manager_s *job)
{
    job->running = 0;

    pthread_join(job->thread, NULL);
}

/* Calculate an fft and send it to file */
void dft_to_file(int *in)
{
    int i;
    FILE *stream;

    double reals[DFT_OUT_LEN];
    double imags[DFT_OUT_LEN];

    dft_wrap(in, reals, imags);

    stream = fopen(CONF_FFT, "w");
    fprintf(stream, "{\"fft\": {\n");

    for (i = 0; i < DFT_OUT_LEN; ++i)
    {
        if (i) fprintf(stream, ",\n");
        fprintf(stream, "    %.2f: %.2f",
            (i + 1) * (DFT_MAX_FREQ/DFT_OUT_LEN),
            sqrt(reals[i] * reals[i] + imags[i] * imags[i])
        );
    }

    fprintf(stream, "\n}}\n");
    fclose(stream);
}

/* The main thread for the manager thread */
static void *xcorr_manager_main(void *arg)
{
    packet_s *pkt;
    FILE *f;
    xcorr_manager_s *job;
    xcorr_job_s     *workers;
    int              njob;

    job     = arg;
    workers = job->workers;
    pkt     = job->packet;

    /* This is set when the last xcorr was part of a calibration */
    job->calibratingstarted = 0;
    /* This is set by the main thread */
    job->calibrating        = 0;

    memset(job->calib, 0, sizeof(job->calib));
    job->ncalib = 1;

    // When this is working fully, we don't need to mkfifo!
    //mkfifo("/tmp/chinchilla-serial", 0666);
    f = fopen(, "r");

    /* Initialize the xcorrelation workers */
    for (njob = 0; njob < NUM_XCORR; ++njob)
        xcorr_job_init(
            &(workers[njob]),
            pkt->data[0], pkt->data[1 + njob], pkt->xcorr[njob]
        );

    while (job->running)
    {
        int ind;

        if (sample_packet_recv(pkt, f) != 0)
        {
            usleep(100000);
            continue;
        }

        for (ind = 0; ind < NUM_MICS; ++ind)
            xcorr_norm(pkt->data[ind]);

        for (njob = 0; njob < NUM_XCORR; ++njob)
            xcorr_job_launch(&(workers[njob]));

        dft_to_file(pkt->data[0]);

        for (njob = 0; njob < NUM_XCORR; ++njob)
            xcorr_job_wait(&(workers[njob]));

        // This is the case where calibrating has just been started.
        if (job->calibrating && !job->calibratingstarted)
        {
            puts("Starting calibration");
            job->calibratingstarted = 1;
            job->ncalib             = 0;
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
                printf("%d NCALIB\n", job->ncalib);
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
                    printf("%d %d %d %d\n", xc, ind, job->calib[xc][ind],
pkt->xcorr[xc][ind]);
                    pkt->xcorr[xc][ind] -= job->calib[xc][ind];
                }
            }
            sample_match_peaks(pkt);
        }
    }

    for (njob = 0; njob < NUM_XCORR; ++njob)
        xcorr_job_kill(&(workers[njob]));

    fclose(f);

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
