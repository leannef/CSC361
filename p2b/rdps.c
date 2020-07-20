#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <termios.h>
#include <errno.h>
#include "rdp_header.h"

char buffer[MAX_BUFFER][MAXBUFFER];
short bufferSize;
int bufferInitByte, bufferPacketCount;
int winErrors;
const char * filename;
int currentByte;
float timeDur = 0;
char recvlastACK = 0;

int sock;
struct sockaddr_in server;
struct sockaddr_in client;
socklen_t clientAddrSize = sizeof(client);
socklen_t serverAddrSize = sizeof(server);

/* This function print log info once receive/send segment */
void printMessage(char eventtype, char* packet){
    	char type[4];
    	int iSeq;
    	short payload, windowsize;
    	char sSeq[11];
    	char payload_len[6], window_len[6];

	char curTime[30];
	struct timeval tv;
	time_t curtime;
    	gettimeofday(&tv, NULL); 
    	curtime=tv.tv_sec;
    	strftime(curTime,30,"%T.",localtime(&curtime));

	//HH:MM:SS.us event_type sip:spt dip:dpt packet_type seqno/ackno length/window
   	printf("%s%ld ", curTime, tv.tv_usec);
    	printf("%c ", eventtype);
    	printf("%s:%d ", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
	printf("%s:%d ", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
    	strcpy(type, packet + 7);
   	printf("%s ", packet + 7);
    	memcpy((char*)&iSeq, packet + 11, 4);
    	sprintf(sSeq, "%d", iSeq);
    	printf("%s %s ", packet + 7, sSeq);
    	memcpy((char*)&payload, packet + 15, 2);
    	sprintf(payload_len, "%d", payload);
    	printf("%s/", payload_len);
    	memcpy((char*)&windowsize, packet + 17, 2);
    	sprintf(window_len, "%d", windowsize);
    	printf("%s\n", window_len);
    	if (strcmp(type, "ACK") == 0) ackrecv++;
    	else if (strcmp(type, "FIN") == 0) finsent++;
    	else if (strcmp(type, "SYN") == 0) synsent++;
    	else if (strcmp(type, "RST") == 0) {
        	if (eventtype == 'r') rstrecv++;
        	else rstsent++;
    	}
    	else if (strcmp(type, "DAT") == 0) {
        	switch (eventtype) {
           	case 's':
                	totalBytes += payload;
                	uniqueBytes += payload;
                	totalPacket++;
                	uniquePacket++;
                	break;
            	case 'S':
                	totalBytes += payload;
                	totalPacket++;
                	break;
        	}
   	 }

}

void printInfo() {

    	printf("total data bytes sent: %d\n", totalBytes);
    	printf("unique data bytes sent: %d\n", uniqueBytes);
   	printf("total data packets sent: %d\n", totalPacket);
    	printf("unique data packets sent: %d\n", uniquePacket);
    	printf("SYN packets sent: %d\n", synsent);
    	printf("FIN packets sent: %d\n", finsent);
    	printf("RST packets sent: %d\n", rstsent);
    	printf("ACK packets received: %d\n", ackrecv);
    	printf("RST packets received: %d\n", rstrecv);
    	printf("total time duration (second): %f\n", timeDur);
}


// Adapted from: http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash(char * packet) {
    	unsigned long hash = 5381;
    	short length;
    	memcpy((char*)&length, packet + 15, 2);
    	int totalLength = 19 + length, i;
    	char c;
    	for (i = 0; i < totalLength; i++) {
        	c = packet[i];
        	hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    	}
    	return hash;
}


void setLog(){
    	strcpy(clientArray, clientPacket.magic);
    	strcpy(clientArray+7, clientPacket.type);
    	memcpy(clientArray+11, (char*)&(clientPacket.synack_number), 4);
    	memcpy(clientArray+15, (char*)&(clientPacket.length), 2);
    	memcpy(clientArray+17, (char*)&(bufferSize), 2);
    	memcpy(clientArray+19, clientPacket.data, clientPacket.length);
    	clientPacket.checksum = hash(clientArray);
    	memcpy(clientArray+19+MAX_DATA, (char*)&(clientPacket.checksum), sizeof(unsigned long));
}

void resendPacket(int bufferIndex){
    	int ack;
    	memcpy ((char*)&ack, buffer[bufferIndex]+11, 4);
    	printMessage('S', buffer[bufferIndex]);
    	sendto(sock, buffer[bufferIndex], MAXBUFFER, 0, (struct sockaddr *) &server, serverAddrSize);
}

void clearBuffer(){
    	int i;
    	int minusOne = -1;
    	for (i = 0; i < bufferSize; i++){
        	memset(buffer[i], '\0', MAXBUFFER);
        	memcpy(buffer[i]+11, (char*)&minusOne, 4);
    	}
    	winErrors = 0;
}



void sendPacket(){
    	printMessage('s', clientArray);
    	sendto(sock, clientArray, MAXBUFFER, 0, (struct sockaddr *) &server, serverAddrSize);
}

void recvPacket(){
	if(recvfrom(sock,serverArray,MAXBUFFER,0,(struct sockaddr *)&server, &serverAddrSize)<0){
		perror("error: send message failed\n");
		exit(EXIT_FAILURE);
	}
    	strcpy(serverPacket.magic, serverArray);
    	strcpy(serverPacket.type, serverArray+7);
    	memcpy((char*)&(serverPacket.synack_number), serverArray+11, 4);
    	memcpy((char*)&(serverPacket.length), serverArray+15, 2);
    	memcpy((char*)&(serverPacket.window), serverArray+17, 2);
    	memcpy((char*)&(serverPacket.data), serverArray+19, serverPacket.length);
    	memcpy((char*)&(serverPacket.checksum), serverArray+19+MAX_DATA, sizeof(unsigned long));
}

void sendSYN(){
    	strcpy(clientPacket.magic, "CSC361");
    	strcpy(clientPacket.type, "SYN");
	//generate random number
	time_t t;
	srand((unsigned) time(&t));
	int ramNum = rand() %10;

   	clientPacket.synack_number = ramNum;
    	setLog();
    	sendPacket();
}

char waitForDATACK(){
    	int maxAck = 0;
    	fd_set set, reads_fds;
   	struct timeval timeout;
    	int result;
    	FD_ZERO (&reads_fds);
    	FD_SET (sock, &reads_fds);
    	timeout.tv_sec = 4;
    	timeout.tv_usec = 0;
    	while (1){
       
        	timeout.tv_sec = 1;
        	timeout.tv_usec = 0;
        	set = reads_fds;
        	result = select(sock+1, &set, NULL, NULL, &timeout);
        	if (result == 0) {
		   //time out
            		int i;
            		int ack;
          		for (i = 0; i < bufferSize; i++)  {
                		memcpy((char*)&ack, buffer[i]+11, 4);
                		if (ack >= maxAck) resendPacket(i);
            		}
            		winErrors++;

        	}else if (result == 1)  {           
            		unsigned long cs;
			recvPacket();
			cs = hash(serverArray);
			if (cs != serverPacket.checksum) {
				continue;
			} else {
			    }
            		if (strcmp(serverPacket.type, "ACK") == 0){
            
                		int i;
                		int ack;

                	// ackno = 0,attemping finish
               	 	if (serverPacket.synack_number == 0) {
                    		printMessage('r', serverArray);
                    		recvlastACK = 1;
                    		break;
                	}

                	if (serverPacket.synack_number > maxAck) {   
                    		maxAck = serverPacket.synack_number;
                    		printMessage('r', serverArray);
                	} else {
                    		// We've seen it before.
                    		printMessage('R', serverArray);
                	}

                		ack = -1;
                		i = bufferSize-1;
                		while (ack < 0){
                    			memcpy((char*)&ack, buffer[i]+11, 4);
                    			i--;
                		}
                		if (serverPacket.synack_number > ack) {
       
                    		break;
                	}
            		} else if (strcmp(serverPacket.type, "RST") == 0){
                		printf("Reset received.\n");
                		return 0;
            		} else {
                		// resend
                		printf("Error, sending RST\n");
                		strcpy(clientPacket.magic, "CSC361");
    				strcpy(clientPacket.type, "RST");
   				clientPacket.synack_number = 0;
    				setLog();
    				sendPacket();
                		return 0;
           		}
        	}
    	}
    	return 1;
}


void transfering(){
    	bufferInitByte = 0;
    	bufferPacketCount = 0;
    	clearBuffer();
    	FILE * fp;
	fp = fopen(filename, "r");
    	int currentByte = 0, bytes = 0;
    	strcpy(clientPacket.type, "DAT");
	char abort = 0;
    	while (!feof(fp)){
        bytes = fread(clientPacket.data, 1, MAX_DATA, fp);
        	if (bytes > 0) {
            		// send the packet
            		clientPacket.synack_number = currentByte;
            		currentByte += bytes;
            		clientPacket.length = bytes;
            		setLog();
               		sendPacket();
            		// copy packet content into buffer
            		memcpy(buffer[bufferPacketCount], clientArray, MAXBUFFER);
            		bufferPacketCount++;
			// wait for ACK if buffer is full
           		if (bufferPacketCount == bufferSize || bytes < MAX_DATA){
                	if (!waitForDATACK()){
                    	// if the ACK failed, abort.
                    		abort = 1;
                    		break;
                	}else{
                    		//  send the next window if received final ACK.
                    		if (winErrors == 0){
                        		bufferSize += 10;
                    		}else if (winErrors > bufferSize / 2){
                        		bufferSize -= 10;
                    		}
                    		if (bufferSize > MAX_BUFFER) bufferSize = MAX_BUFFER;
                    		if (bufferSize < MIN_BUFFER) bufferSize = MIN_BUFFER;
                    			bufferInitByte = currentByte;
                   			bufferPacketCount = 0;
                   			clearBuffer();
                	}
          		}
        	}
    	}
    	fclose(fp);
    	if (!abort){
		strcpy(clientPacket.magic, "CSC361");
    		strcpy(clientPacket.type, "FIN");
    		clientPacket.synack_number = currentByte;
    		setLog();
    		sendPacket();
        	waitForDATACK();
        	if (!recvlastACK) {
    			recvPacket();
    			printMessage('r', serverArray);
	}
    }
}

int main(int argc, char * argv[]){
    	struct timespec start, end;
    	uint64_t delta_us;
    	// Ensure proper command line arguments.
    	if (argc < 6) {
       		printf("usage: %s [sender_ip] [sender_port] [receiver_ip] [receiver_port] [sender_file_name]\n", *argv);
        	exit(EXIT_FAILURE);
    	}
	
    	filename = argv[5];
    	bufferSize = 5;
    	sock = socket(AF_INET, SOCK_DGRAM, 0);

    	bzero((char *)&client, sizeof(client));
    	client.sin_family = AF_INET;
    	client.sin_port = htons(atoi(argv[2]));
    	client.sin_addr.s_addr = inet_addr(argv[1]);
    	//inet_pton(AF_INET, argv[1], &(client.sin_addr));
    	memset(client.sin_zero, 0, sizeof client.sin_zero);

    	int optval = 1;
    	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)& optval, sizeof( optval)) < 0){
        	printf("Opt error\n");
    	}
    	bind(sock, (struct sockaddr *) &client, sizeof(client));

    	server.sin_family = AF_INET;
    	server.sin_port = htons(atoi(argv[4]));
    	inet_pton(AF_INET, argv[3], &(server.sin_addr));
    	memset(server.sin_zero, 0, sizeof server.sin_zero);

    	clock_gettime(CLOCK_MONOTONIC, &start);
	//establish connection
    	sendSYN();
    	recvPacket();
    	printMessage('r', serverArray);
    	transfering();
    	clock_gettime(CLOCK_MONOTONIC, &end);
    	delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    	timeDur = delta_us / 1000000.0;
    	printInfo();
    	return 0;

}
