#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "sound.h"

static uint64_t get_time_ms()
{
    uint64_t rtn;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    rtn = 1000 * tv.tv_sec;
    rtn += tv.tv_usec / 1000;

    return rtn;
}

void sound_print(sound_s *sound, FILE *stream)
{
    static int id = 1;
    int nchrs = 1024;
    char buf[nchrs];
    char *ptr = &buf[0];
    char *end = &buf[nchrs];
    uint64_t time;

    ptr += snprintf(ptr, end - ptr, "{\"id\": %d, ",     id++);
    ptr += snprintf(ptr, end - ptr, "\"angle\": %f, ",   sound->angle);
    ptr += snprintf(ptr, end - ptr, "\"amplitude\": %f, ", sound->amplitude);
    ptr += snprintf(ptr, end - ptr, "\"freq\": null, ");
    ptr += snprintf(ptr, end - ptr, "\"speed\": %f, ", get_sound_speed(sound));
    ptr += snprintf(ptr, end - ptr, "\"error\": %f, ", get_sound_error(sound));
    ptr += snprintf(ptr, end - ptr, "\"time\": %ld }\n",   get_time_ms());

    if (ptr >= end)
        return;

    fwrite(buf, 1, ptr - buf, stream);
}


FILE *sound_trim_file(const char *fname)
{
    FILE *filein;
    FILE *fileout;

    const int maxsize = 4096, trimsize = 1024;

    char file[trimsize];
    struct stat status;
    char *end, *iter;

    if (access(fname, F_OK))
        return NULL;

    if (stat(fname, &status))
        return NULL;

    if (status.st_size <= maxsize)

        return NULL;

    filein = fopen(fname, "r");
    fseek(filein, status.st_size - trimsize, SEEK_SET);
    fread(file, 1, trimsize, filein);
    fclose(filein);

    iter = &file[0];
    end  = &file[trimsize - 1];
    while (iter < end)
    {
        if (*(++iter) == '\n')
        {
            fileout = fopen(fname, "w");
            fwrite(iter + 1, 1, iter - end, stdout);
            fwrite(iter + 1, 1, iter - end, fileout);
            return fileout;
        }
    }

    return NULL;
}

FILE *sound_get_file(void)
{
    FILE *rtn;
    char *fname = "chinchilla-sounds";

    rtn = sound_trim_file(fname);
    if (!rtn)
        return fopen(fname, "a");

    return rtn;
}

bool sound_verify(sound_s *sound)
{
    double speed = get_sound_speed(sound);
    double error = get_sound_error(sound);

    return (error < 0.2e-3) && (speed > 300.0) && (speed < 400.0);
}

bool sound_init(sound_s *sound, double dt0, double dt1, double dt2, int v)
{
    sound->dt[0] = dt0;
    sound->dt[1] = dt1;
    sound->dt[2] = dt2;

    if (!sound_verify(sound))
        return false;

    sound->angle = get_sound_angle(sound);
    sound->amplitude = v;

    return true;
}

bool sound_match_peaks(
    sound_s *sound,
    double *dt0, int ndt0, int *v0,
    double *dt1, int ndt1, int *v1,
    double *dt2, int ndt2, int *v2)
{
    int i0, i1, i2;
    FILE *f;

    f = sound_get_file();

    for (i0 = 0; i0 < ndt0; ++i0)
    for (i1 = 0; i1 < ndt1; ++i1)
    for (i2 = 0; i2 < ndt2; ++i2)
    {
        sound_s sound;
        if (sound_init(
            &sound,
            dt0[i0], dt1[i1], dt2[i2],
            v0[i0] + v1[i1] + v2[i2]))
        {
            sound_print(&sound, f);
        }
    }

    fclose(f);
}
