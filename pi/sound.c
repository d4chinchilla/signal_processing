#include <stdio.h>
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
    ptr += snprintf(ptr, end - ptr, "\"amplitude\": %f", sound->amplitude);
	ptr += snprintf(ptr, end - ptr, "\"freq\": null, ");
	ptr += snprintf(ptr, end - ptr, "\"time\": %ld }",   get_time_ms());
	
	if (ptr >= end)
		return;
	
	fwrite(buf, 1, ptr - buf, stream);
}

bool sound_verify(sound_s *sound)
{
	double speed = get_sound_speed(sound);
	double error = get_sound_error(sound);

	return (error < 0.2e-3) && (speed > 300.0) && (speed < 500.0);
}

bool sound_init(sound_s *sound, double dt0, double dt1, double dt2)
{
	sound->dt[0] = dt0;
	sound->dt[1] = dt1;
	sound->dt[2] = dt2;
	
	if (!sound_verify(sound))
		return false;
	
	sound->angle = get_sound_angle(sound);
}

bool sound_match_peaks(sound_s *sound, double *dt0, int ndt0, double *dt1, int ndt1, double *dt2, int ndt2)
{
	while (--ndt0) while (--ndt1) while (--ndt2)
	{
		sound_s sound;
		if (sound_init(&sound, dt0[ndt0], dt1[ndt1], dt2[ndt2]))
		{
			sound_print(&sound, stdout);
		}
	}
}