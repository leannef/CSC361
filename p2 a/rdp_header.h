#ifndef RDP_HEADER_H
#define RDP_HEADER_H

#define BUFFER_SIZE 1024

#define MAGIC "CSC361"
#define event_send "s"
#define event_resent "S"
#define event_receive "r"
#define event_receiveAgain "R"

#define ACK 0
#define DAT 1
#define FIN 2
#define RST 3
#define SYN 4

#define RDP_TYPE_COUNT 5
#define MAX_BUFFER_LEN 1024


// RDP packet contents.
struct packet_field {
	char *magic;
   	int type;
    	int seqno;
	int ackno;
	int length;
	int windowSize;
	char *empty_line;
};

struct packet_field packet;


/*
static const char *packet_types[RDP_TYPE_COUNT] = {
    "ACK",
    "DAT",
    "FIN",
    "RST",
    "SYN"
};
*/
//char ACK, SYN, DAT, FIN, RST;

#endif
