#include "sample.h"
#include "xcorr.h"

void main(void)
{
    xcorr_manager_s manager;
    xcorr_manager_init(&manager);
    sleep(1);
    xcorr_manager_kill(&manager);
}
