#include "xcorr.h"
#include "sample.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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
    xcorr_manager_s *job;
    xcorr_job_s     *workers;

    job     = arg;
    workers = job->workers;
    pkt     = job->packet;

    while (job->running)
    {
        int njob;

        if (sample_packet_recv(pkt, NULL) != 0)
        {
            usleep(100000);
            continue;
        }
        
        for (njob = 0; njob < NUM_XCORR; ++njob)
            xcorr_job_init(
                &(workers[njob]),
                pkt->data[0], pkt->data[1 + njob], pkt->xcorr[njob]
            );
        
        for (njob = 0; njob < NUM_XCORR; ++njob)
            xcorr_job_kill(&(workers[njob]));
    }

    return NULL;
}
