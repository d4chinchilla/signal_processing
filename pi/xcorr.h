#if !defined(XCORR_H)
# define XCORR_H

# include <pthread.h>

# define XCORR_LEN  50
# define XCORR_JOBS 4

typedef struct xcorr_job     xcorr_job_s;
typedef struct xcorr_manager xcorr_manager_s;

struct xcorr_job
{
    int running;
    pthread_t thread;

    // Our daddy
    xcorr_manager_s *manager;

    // Signal for when this waiting job should go!
    pthread_cond_t  go;
    pthread_mutex_t go_mtx;

    int *sample1;
    int *sample2;
    int *result;

    const char *test;
};

struct xcorr_manager
{
    int running;
    pthread_t thread;

    xcorr_job_s workers[4];
    xcorr_job_s *waiting;

    // Signal for when a job is ready to work.
    pthread_cond_t ready;
    pthread_mutex_t ready_mtx;

    // Obtained by a job while it is waiting to go.
    // It's job is to exclude two processes waiting
    //  at once.
    pthread_mutex_t waiting_mtx;
};

void  xcorr_manager_init(xcorr_manager_s *manager);
void  xcorr_manager_kill(xcorr_manager_s *manager);

#endif
