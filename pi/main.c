#include "sample.h"
#include "xcorr.h"
#include "errno.h"
#include "string.h"

void read_ctl(FILE *f)
{
    char line[64];
}

FILE *ctl_file(void)
{
    if (mkfifo("/tmp/chinchilla-backend-ctl", 0666) == -1)
    {
        if (errno != EEXIST)
            printf("Error, cannot make backend-ctl fifo: %s\n", strerror(errno));
    }
    
    f = fopen("/tmp/chinchilla-backend-ctl", "r");
    
    return f;
}

void clean_files(void)
{
    fopen("/tmp/chinchill
}

void main(void)
{	
    int running;
    FILE *ctlf;
    xcorr_manager_s manager;
    xcorr_manager_init(&manager);
    
    running = 1;
    ctlf    = ctl_file();
    while (running)
    {
        char line[16];
        char *chr, *end;
        chr = &line[0];
        end = &line[sizeof(line) - 1];
        
        while (chr < end)
        {
            int cint;
            cint = fgetc(ctlf);
            if (cint == -1) break;
            
            *(chr++) = (unsigned char)cint;
        }

        if (memcmp("stop", line, 4) == 0)
            running = 0;
        
        if (memcmp("calibrate", line, 4) == 0)
        {
            printf("CALIBRATING\n");
            sleep(5);
            printf("DONE\n");
        }
        
        
    }
    fclose(ctlf);

    xcorr_manager_kill(&manager);
}
