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
int bufferSize,  bufferInitByte, bufferPacketCount;

int sock;
struct sockaddr_in client;
struct sockaddr_in server;
socklen_t clientAddrSize = sizeof(client);

const char * filename;
int currentByte, maxACKSent = -1;
char finReceived = 0;
float timeDur = 0;

void printInfo() {
    	printf("total data bytes received: %d\n", totalBytes);
    	printf("unique data bytes received: %d\n", uniqueBytes);
    	printf("total data packets received: %d\n", totalPacket);
    	printf("unique data packets received: %d\n", uniquePacket);
    	printf("SYN packets received: %d\n", synrecv);
    	printf("FIN packets received: %d\n", finrecv);
    	printf("RST packets received: %d\n", rstrecv);
    	printf("ACK packets sent: %d\n", acksent);
    	printf("RST packets sent: %d\n", rstsent);
    	printf("total time duration (second): %f\n", timeDur);
}

// Adapted from: http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash(char * packet) {
    	unsigned long hash = 5381;
    	short length;
    	memcpy((char*)&length, packet + 15, 2);
    	int totalLength = 19 + length;

   	char c;
    	int i;
    	for (i = 0; i < totalLength; i++) {
        	c = packet[i];
        	hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    	}
    	return hash;
}


// print out the log information
void printMessage(char eventtype, char* packet){
    	char type[4], sSeq[11], payload_len[6], window_len[6];
    	int iSeq;
    	short payload, windowsize;

    	char curTime[30];
    	struct timeval tv;
    	time_t curtime;
    	gettimeofday(&tv, NULL); 
    	curtime=tv.tv_sec;
    	strftime(curTime,30,"%T.",localtime(&curtime));

    	//HH:MM:SS.us event_type sip:spt dip:dpt packet_type seqno/ackno length/window
    	printf("%s%ld ", curTime, tv.tv_usec);
    	printf("%c ", eventtype);
    	printf("%s:%d %s:%d ", inet_ntoa(client.sin_addr), ntohs(client.sin_port), inet_ntoa(server.sin_addr), ntohs(server.sin_port));
    	strcpy(type, packet + 7);
    	printf("%s ", packet + 7);
    	memcpy((char*)&iSeq, packet + 11, 4);
    	sprintf(sSeq, "%d", iSeq);
    	printf("%s ", sSeq);

    	memcpy((char*)&payload, packet + 15, 2);
    	sprintf(payload_len, "%d", payload);
    	printf("%s/", payload_len);
    	memcpy((char*)&windowsize, packet + 17, 2);
    	sprintf(window_len, "%d", windowsize);
    	printf("%s\n", window_len);
    	if (strcmp(type, "ACK") == 0) acksent++;
    	else if (strcmp(type, "FIN") == 0) finrecv++;
    	else if (strcmp(type, "SYN") == 0) synrecv++;
    	else if (strcmp(type, "RST") == 0) {
        if (eventtype == 'r') rstrecv++;
        else rstsent++;
    	}else if (strcmp(type, "DAT") == 0) {
        	switch (eventtype) {
            		case 'r':
                	totalBytes += payload;
                	uniqueBytes += payload;
               	 	totalPacket++;
                	uniquePacket++;
                	break;
            	case 'R':
                	totalBytes += payload;
                	totalPacket++;
                	break;
        	}
    	}
}


void clearBuffer(){
    	int i;
    	int minusOne = -1;
    	for (i = 0; i < bufferSize; i++) {
        	memset(buffer[i], '\0', MAXBUFFER);
        	memcpy(buffer[i]+11, (char*)&minusOne, 4);
    	}
}

void sendPacket(){
    	strcpy(serverArray, serverPacket.magic);
    	strcpy(serverArray+7, serverPacket.type);
    	memcpy(serverArray+11, (char*)&(serverPacket.synack_number), 4);
    	memcpy(serverArray+15, (char*)&(serverPacket.length), 2);
    	memcpy(serverArray+17, (char*)&(serverPacket.window), 2);
    	memcpy(serverArray+19, serverPacket.data, serverPacket.length);

    	serverPacket.checksum = hash(serverArray);
    	memcpy(serverArray+19+MAX_DATA, (char*)&(serverPacket.checksum), sizeof(unsigned long));
    	sendto(sock, serverArray, MAXBUFFER, 0, (struct sockaddr *) &client, clientAddrSize);
}

void recvPacket(){
	int recvsize = recvfrom(sock,clientArray,MAXBUFFER,0,(struct sockaddr *)&client, &clientAddrSize);
	if(recvsize<0){
		perror("error: on recvfrom ! \n");
		exit(EXIT_FAILURE);
	}
    	strcpy(clientPacket.magic, clientArray);
    	strcpy(clientPacket.type, clientArray+7);
    	memcpy((char*)&(clientPacket.synack_number), clientArray+11, 4);
    	memcpy((char*)&(clientPacket.length), clientArray+15, 2);
    	memcpy((char*)&(clientPacket.window), clientArray+17, 2);
    	memcpy((char*)&(clientPacket.data), clientArray+19, clientPacket.length);
    	memcpy((char*)&(clientPacket.checksum), clientArray+19+MAX_DATA, sizeof(unsigned long));
}

void transfering(int number){
    	strcpy(serverPacket.magic, "CSC361");
    	strcpy(serverPacket.type, "ACK");
    	serverPacket.synack_number = number;
    	sendPacket();
    	if (finReceived) printMessage('s', serverArray);
    	else {
        	if (number < maxACKSent) printMessage('S', serverArray);
        	else {
            		printMessage('s', serverArray);
            		maxACKSent = number;
        	}
    	}
}

void writeIntoFile(FILE * f, int lastIndex){
   	int i;
    	for (i = 0; i <= lastIndex; i++){
        	short length;
        	memcpy((char*)&length, buffer[i]+15, 2);
        	fwrite(buffer[i]+19, 1, length, f);
    	}
}

void waitForDAT(){
    	int finalPacketSequence = -1, lastAckSent = 0;
    	FILE * fp;
	fp = fopen(filename, "wb");
    	recvPacket();
    	bufferSize = clientPacket.window;
    	clearBuffer();
    	bufferPacketCount = 0;
    	bufferInitByte = 0;
	unsigned long check_sum;
    	while (1) {
        	if (strcmp(clientPacket.type, "DAT") == 0){
           	int lastAck, tmpSize, bufferAck, i;
            	char lost;
            	// check if it is a new window
            	tmpSize = clientPacket.window;
            	if (tmpSize != bufferSize){
                	bufferSize = tmpSize;
                	clearBuffer();
            	}
            	// if synack =-1 it is new packet
            	i = (clientPacket.synack_number - bufferInitByte) / MAX_DATA;
            	if (i < 0 || i >= bufferSize){
                	// wait for next packet
                	transfering(clientPacket.synack_number + MAX_DATA);
                do {
                    	recvPacket();
                    	check_sum = hash(clientArray);
                } while (check_sum != clientPacket.checksum);
                	printMessage('R', clientArray);
                	continue;
            	}
            	memcpy((char*)&bufferAck, buffer[i] + 11, 4);
            	memcpy(buffer[i], clientArray, MAXBUFFER);
           		if (bufferAck < 0) {
                	bufferPacketCount++;
               	 	printMessage('r', clientArray);
            		} else {
                		printMessage('R', clientArray);
            		}

            		// check if ready to send ACK
            		if (clientPacket.synack_number == lastAckSent){
                		short length = clientPacket.length;
               		 	//send ACK
                		lastAckSent = clientPacket.synack_number + length;
                		transfering(lastAckSent);
                		i = i + 1;
                		memcpy((char*)&bufferAck, buffer[i] + 11, 4);
                		while (i < bufferSize && bufferAck >= 0){
                    			memcpy((char*)&length, buffer[i]+15, 2);
                    			lastAckSent = bufferAck + length;
                    			transfering(lastAckSent);
                    			i = i + 1;
                    			if (i < bufferSize) memcpy((char*)&bufferAck, buffer[i] + 11, 4);
                		}

           		}else if (clientPacket.synack_number < lastAckSent)  {
                		//resend, in case it is lost
               			 transfering(clientPacket.synack_number + MAX_DATA);
            		}
			// write content into file
            		if (lastAckSent == finalPacketSequence) {
                		writeIntoFile(fp, i);
                		break;
            		} else if (i == bufferSize) {
                		writeIntoFile(fp, i-1);
                		clearBuffer();
                		bufferPacketCount = 0;
                		bufferInitByte = lastAckSent;
            		}
        	} else if (strcmp(clientPacket.type, "FIN") == 0) {
            		//  keep looping until all packet arrive
            		int i,  ack;
            		char lost = 0;           
            		printMessage('r', clientArray);
            		finalPacketSequence = clientPacket.synack_number;
            	if (finalPacketSequence == lastAckSent) {
                	for (i = 0; i < bufferSize; i++) {
                    		memcpy((char*)&ack, buffer[i]+11, 4);
                    		if (ack < 0) break;
                	}
                	writeIntoFile(fp, i-1);
                	break;
           	}

        }else if (strcmp(clientPacket.type, "RST") == 0){
            	// If reset, send back ACK 
            	printMessage('r', clientArray);
            	break;
        }

        // ignore any bad checksum.
        do {
            	recvPacket();
            	check_sum = hash(clientArray);
        } while (check_sum != clientPacket.checksum);
   	}
    	transfering(0);
    	fclose(fp);

}

int main(int argc, char * argv[]){
    	struct timespec start, end;
    	uint64_t delta_us;
	// Ensure proper command line arguments.
    	if (argc != 4) {
    		printf("usage: %s [receiver_ip] [receiver_port] [filename]\n", *argv);
        	exit(EXIT_FAILURE);
    	}
    	filename = argv[3];
    	sock = socket(AF_INET, SOCK_DGRAM, 0);
    	server.sin_family = AF_INET;
    	server.sin_port = htons(atoi(argv[2]));
    	server.sin_addr.s_addr = htonl(INADDR_ANY);
    	memset(server.sin_zero, '\0', sizeof server.sin_zero);
    	bind(sock, (struct sockaddr *) &server, sizeof(server));

    	bufferSize = 5;
   	while (1){
		// receive SYN from sender
        	recvPacket();
    		printMessage('r', clientArray);
    		currentByte = clientPacket.synack_number;
        	clock_gettime(CLOCK_MONOTONIC, &start);
        	transfering(clientPacket.synack_number);
        
		clearBuffer();
        	waitForDAT();
        	clock_gettime(CLOCK_MONOTONIC, &end);
        	delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        	timeDur = delta_us / 1000000.0;
        	printInfo();
    	}
    	return 0;
}
