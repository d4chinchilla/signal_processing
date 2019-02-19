#include "xcorr.h"
#include <stdio.h>
#include <unistd.h>

static void  xcorr_job_init(xcorr_job_s *job, xcorr_manager_s *manager);
static void  xcorr_job_kill(xcorr_job_s *job);
static void *xcorr_job_main(void *arg);

static void *xcorr_manager_main(void *arg);

static void xcorr_job_init(xcorr_job_s *job, xcorr_manager_s *manager)
{
    job->running = 1;
    job->manager = manager;

    pthread_mutex_init(&(job->go_mtx), NULL);
    pthread_cond_init(&(job->go), NULL);

    pthread_create(&(job->thread), NULL, xcorr_job_main, job);
}

static void xcorr_job_kill(xcorr_job_s *job)
{
    job->running = 0;
    pthread_join(job->thread, NULL);
}

static void *xcorr_job_main(void *arg)
{
    xcorr_job_s     *job;
    xcorr_manager_s *manager;
    job     = arg;
    manager = job->manager;
    pthread_mutex_lock(&(job->go_mtx));

    printf("Job %d\n", job -manager->workers);

    while (job->running)
    {
        pthread_mutex_lock(&(manager->waiting_mtx));

        // Setup manager
        manager->waiting = job;

        printf("Worker ready\n");
        pthread_mutex_lock(&(manager->ready_mtx));
        pthread_cond_signal(&(manager->ready));
        pthread_mutex_unlock(&(manager->ready_mtx));

        printf("Waiting for go\n");
        pthread_cond_wait(&(job->go), &(job->go_mtx));

        pthread_mutex_unlock(&(manager->waiting_mtx));

        // Do work
        printf("Job %d: %s\n", job - manager->workers, job->test);

    }

    return NULL;
}

void xcorr_manager_init(xcorr_manager_s *job)
{
    job->running = 1;
    pthread_mutex_init(&(job->ready_mtx), NULL);
    pthread_cond_init(&(job->ready), NULL);

    pthread_mutex_init(&(job->waiting_mtx), NULL);

    pthread_create(&(job->thread), NULL, xcorr_manager_main, job);
}

void xcorr_manager_kill(xcorr_manager_s *job)
{
    job->running = 0;

    pthread_join(job->thread, NULL);
}

static void *xcorr_manager_main(void *arg)
{
    xcorr_manager_s *job;
    xcorr_job_s     *workers;

    int testind = 0;
    char *tests[] = { "apples", "cakes", "bananas", "wow", "cattlegrid", "meow", "butterfly", "aaaa" };

    job = arg;
    workers = job->workers;

    pthread_mutex_lock(&(job->ready_mtx));

    int njob;
    for (njob = 0; njob < XCORR_JOBS; ++njob)
        xcorr_job_init(&workers[njob], job);

    while (job->running)
    {
        printf("Waiting for worker\n");
        pthread_cond_wait(&(job->ready), &(job->ready_mtx));
        printf("Worker here!\n");

        // Setup JOB

        printf("Dispatched %d: %s\n", job->waiting - workers, tests[testind]);

        job->waiting->test = tests[testind];
        testind += 1;
        if (testind > 7) testind = 0;
        pthread_mutex_lock(&(job->waiting->go_mtx));
        pthread_cond_signal(&(job->waiting->go));
        pthread_mutex_unlock(&(job->waiting->go_mtx));
    }

    for (njob = 0; njob < XCORR_JOBS; ++njob)
        xcorr_job_kill(&workers[njob]);

    return NULL;
}

int main(void)
{
    xcorr_manager_s m;
    xcorr_manager_init(&m);
    sleep(100);
    return 0;
}
