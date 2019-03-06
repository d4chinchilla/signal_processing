#if !defined(SAMPLE_H)
# define SAMPLE_H
# include <stdio.h>

# define SAMPLE_SIZE 1024
# define XCORR_LEN   151
# define NUM_MICS    4
# define NUM_XCORR   (NUM_MICS - 1)
# define MAX_PEAKS   4
# define SAMPLE_RATE 60000

// This is big, so avoid storing it on stack memory as much as possible :)
typedef struct packet packet_s;

struct packet
{
    int data[NUM_MICS][SAMPLE_SIZE];
    int xcorr[NUM_XCORR][XCORR_LEN];
};

int sample_packet_recv(packet_s *pkt, FILE *stream);

int sample_match_peaks(packet_s *pkt);

#endif
