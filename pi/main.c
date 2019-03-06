#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include "sample.h"
#include "xcorr.h"
#include "errno.h"
#include "string.h"

FILE *ctl_file(void)
{
    FILE *f;

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
    FILE *f;
    unlink("/tmp/chinchilla-backend-ctl");
    f = fopen("/tmp/chinchilla-sounds", "w");
    fwrite("", 1, 0, f);

//    fopen("/tmp/chinchill
}

void main(void)
{
    int running;
    FILE *ctlf;
    xcorr_manager_s manager;
    xcorr_manager_init(&manager);

//    system("stty -F " CONF_INPUT " 406:0:18b4:8a30:3:1c:7f:15:4:2:64:0:11:13:1a:0:12:f:17:16:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0");
    running = 1;
    ctlf    = ctl_file();
    while (running)
    {
        char line[16];
        char *chr, *end;
        chr = &line[0];
        end = &line[sizeof(line) - 1];
        memset(line, 0, sizeof(line));

        while (chr < end)
        {
            int cint;
            cint = fgetc(ctlf);
            printf("%d\n", cint);
            if (cint == -1)
            {
                clearerr(ctlf);
                usleep(100000);
                break;
            }

            *(chr++) = (unsigned char)cint;
        }

        if (memcmp("stop", line, 4) == 0)
            running = 0;

        if (memcmp("calibrate", line, 4) == 0)
        {
            manager.calibrating = 1;
            printf("CALIBRATING\n");
            sleep(5);
            printf("DONE\n");
            manager.calibrating = 0;
        }

    }
    fclose(ctlf);

    xcorr_manager_kill(&manager);
    clean_files();
}
