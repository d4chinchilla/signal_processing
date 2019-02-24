#if !defined(SOUND_H)
# define SOUND_H
# include "sample.h"
# include "xcorr.h"
# include <math.h>

typedef struct sound sound_s;

struct sound
{
    double angle;
    double amplitude;
    double dt[NUM_XCORR];
};

/* 1 o----o 2
 *   |    |
 * 3 o----o 4
 *
 *    |--> x
 *  y v
 *
 * dt[0] is xcorr of 1 and 2,
 * dt[1] is xcorr of 1 and 3,
 * dt[2] is xcorr of 1 and 4
 */

#define SOUND_DT_X1(s) (s->dt[0])
#define SOUND_DT_X2(s) (s->dt[2] - s->dt[1])
#define SOUND_DT_Y1(s) (s->dt[1])
#define SOUND_DT_Y2(s) (s->dt[2] - s->dt[0])


/* Get the average delay of the sound in the y direction as it passes *
 * the mics.                                                          */
inline float get_sound_dy(sound_s *sound)
{
    return (SOUND_DT_Y1(sound) + SOUND_DT_Y2(sound)) / 2.0;
}

/* Get the average delay of the sound in the x direction as it passes *
 * the mics.                                                          */
inline float get_sound_dx(sound_s *sound)
{
    return (SOUND_DT_X1(sound) + SOUND_DT_X2(sound)) / 2.0;
}

/* Get the error in the sound. This is how far the sound deviates *
 * from the expected uniform x velocity and uniform y velocity.   *
 * Large values mean either the sound is close, or that this is   *
 * not a sound.                                                   */
inline float get_sound_error(sound_s *sound)
{
    double x1, x2, xerr;
    double y1, y2, yerr;

    x1 = SOUND_DT_X1(sound);
    x2 = SOUND_DT_X2(sound);
    y1 = SOUND_DT_Y1(sound);
    y2 = SOUND_DT_Y2(sound);

    xerr = fabs((x1 - x2));
    yerr = fabs((y1 - y2));

    return xerr + yerr;
}
/* Get the angle of the sound from -pi to +pi */
inline float get_sound_angle(sound_s *sound)
{
    return atan2(get_sound_dy(sound), get_sound_dx(sound));
}

/* Estimate the speed of the sound in m/s */
inline float get_sound_speed(sound_s *sound)
{
    /* The distance between the pairs of mics */
    float mic_dist = 0.2;
    return mic_dist/sqrt(pow(get_sound_dx(sound), 2) + pow(get_sound_dy(sound), 2));
}

#endif
