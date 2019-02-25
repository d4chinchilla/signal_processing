#if !defined(XCORR_H)
# define XCORR_H
# include <pthread.h>
# include "sample.h"

typedef struct xcorr_job     xcorr_job_s;
typedef struct xcorr_manager xcorr_manager_s;

struct xcorr_job
{
    pthread_t thread;

    int *a, *b;
    int *res;
};

struct xcorr_manager
{
    int running;
    pthread_t thread;

    xcorr_job_s workers[NUM_XCORR];
    packet_s *packet;
};

void  xcorr_manager_init(xcorr_manager_s *manager);
void  xcorr_manager_kill(xcorr_manager_s *manager);
int xcorr_next_peak(int *vals, int prev);

#endif
