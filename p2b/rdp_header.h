#ifndef RDP_HEADER_H
#define RDP_HEADER_H

#define MAXBUFFER 1024
#define MAX_DATA 900
#define MAX_BUFFER 100
#define MIN_BUFFER 5

int totalBytes = 0;
int uniqueBytes = 0;
int totalPacket = 0;
int uniquePacket = 0;
int synrecv = 0;
int finrecv = 0;
int rstrecv = 0;
int acksent = 0;
int rstsent = 0;

int synsent = 0;
int finsent = 0;

int ackrecv = 0;

char clientArray[MAXBUFFER], serverArray[MAXBUFFER];

// RDP packet contents.
typedef struct packet_field
{
    char magic[7];
    char type[4];
    int synack_number;
    short length;
    short window;
    char data[MAX_DATA];
    unsigned long checksum;
} packet_field;

packet_field serverPacket;
packet_field clientPacket;

#endif
