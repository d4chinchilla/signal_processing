#if !defined(SAMPLE_H)
# define SAMPLE_H

#define SAMPLE_SIZE 1024

// This is big, so avoid storing it on stack memory as much as possible :)
typedef struct packet packet_s;
struct packet
{
    int data[4][SAMPLE_SIZE];
};

#endif
