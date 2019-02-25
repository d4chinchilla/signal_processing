#include "sample.h"
#include "xcorr.h"

void read_ctl(FILE *f)
{
    char line[64];
}

void main(void)
{
    xcorr_manager_s manager;
    xcorr_manager_init(&manager);
    sleep(1);
    xcorr_manager_kill(&manager);
}
