#if !defined(XCORR_H)
# define XCORR_H
# include <pthread.h>
# include "sample.h"
# include "conf.h"

#define PEAK_X_THRESHOLD 20
#define PEAK_Y_THRESHOLD 0

typedef struct xcorr_job     xcorr_job_s;
typedef struct xcorr_manager xcorr_manager_s;

struct xcorr_job
{
    pthread_t thread;

    int running;
    pthread_cond_t  launch;
    pthread_mutex_t launch_mtx;
    pthread_cond_t  done;
    pthread_mutex_t done_mtx;

    int *a, *b;
    int *res;
};

struct xcorr_manager
{
    int running;
    pthread_t thread;
    int calibrating;
    int calibratingstarted;

    int calib[NUM_XCORR][XCORR_LEN];
    int ncalib;
    xcorr_job_s workers[NUM_XCORR];
    packet_s *packet;
};

void  xcorr_manager_init(xcorr_manager_s *manager);
void  xcorr_manager_kill(xcorr_manager_s *manager);
int xcorr_next_peak(int *vals, int prev);

#endif
