#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include "rdp_header.h"
#include <arpa/inet.h>

#define MAX_STR_LEN 50


char buffer[1024], recBuffer[1024],tmpBuffer[1024];
int synrecv = 0;
int finrecv = 0;
int ackrecv = 0;
int synsent = 0;
int finsent = 0;
int acksent = 0;

//Client
void printMessage(){
	printf("total data bytes sent: \n");
	printf("unique data bytes sent: \n");
	printf("total data packets sent: \n");
	printf("unique data packets sent: \n");
	printf("SYN packets sent: \n");
	printf("FIN packets sent: \n");
	printf("RST packets sent: 0 \n");
	printf("ACK packets received: \n");
	printf("RST packets received: 0 \n");
	printf("total time duration (second):  \n");
}	

void sendSYN(int sockfd, struct sockaddr_in recv_addr, char *buffer1, socklen_t rec_len ){
	int result;
	packet.type =SYN;
	buffer1[0] = packet.type;
	//printf("buffer1: %s\n", buffer1);
        if ((result = sendto(sockfd, buffer1, strlen(buffer1), 0,
	 (struct sockaddr*)&recv_addr, rec_len)) < 0 ) {
		perror("error: send message failed\n");
		exit(EXIT_FAILURE);
	}
	printf("recv_addr: %s, content: %s\n",  inet_ntoa(recv_addr.sin_addr), buffer1);
	memset(buffer1, 0, sizeof(buffer1)*sizeof(char));			
	synsent += 1;
	

	//return 2;
}

void sendToReceiver(int sockfd, struct sockaddr_in recv_addr, char *buffer2, socklen_t rec_len ){
	int result;
	packet.type = DAT;
	memset(tmpBuffer, 0, sizeof(tmpBuffer)*sizeof(char));				
	tmpBuffer[0] = packet.type;
	strcat(tmpBuffer, buffer2);
	//printf("buffer2 : %s \n", tmpBuffer);
        if ((result = sendto(sockfd, tmpBuffer, sizeof(tmpBuffer), 0,
	 (struct sockaddr*)&recv_addr, rec_len)) < 0) {		
		perror("error: send message failed\n");
		exit(EXIT_FAILURE);
	}
	printf("packet data sended to receiver: %s\n", tmpBuffer);
	memset(tmpBuffer, 0, sizeof(tmpBuffer)*sizeof(char));				
	
}

void openFile(char *filename, char *fileBuffer){
	int bytes_read;
	FILE *fp;
	fp = fopen(filename, "r");
	if( fp == NULL){
		printf("Erorr: No Such file Exists \n");
		exit(EXIT_FAILURE);
	}else{
		fseek (fp, 0L, SEEK_END);
		long int file_size;
		file_size = ftell(fp);
		char *file =(char *) malloc(file_size+1);
		fseek(fp, 0L, SEEK_SET);
		bytes_read = fread(file, sizeof(char), file_size, fp);	
		fileBuffer[bytes_read];
		strcpy(fileBuffer, file);
		fclose(fp);	
		free(file);
	}
}

/*
void handleHandShake2() {
	int result;
	if ((numbytes_rec = recvfrom(sockfd, rec_buffer,
	 sizeof(rec_buffer), 0,
	 (sockaddr *) &rec_addr, &rec_len)) == -1) {
		error("on recvfrom()!");
	}

	rdp_segment rec_seg = generate_segment(rec_buffer);
	if (rec_seg.RST == true) {event = 'S'; state = Sync;
		log_event('r', "RST", rec_seg, sender_addr, rec_addr );
	}
	else if (rec_seg.SYN == true && rec_seg.ackno == isn+1) {
		log_event('r', "SYN+ACK", rec_seg, sender_addr, rec_addr );
		//debugprint("HandShake2 Received");
		//print_rdp_message(rec_seg);
		// send HS 3 now
		rdp_segment resp_seg = rec_seg;
		resp_seg.seqno = rec_seg.ackno+1;
		resp_seg.ackno = rec_seg.seqno+1;
		resp_seg.SYN = false;
		rdp_payload_set(resp_seg, "HSP3");
		strcpy(send_buffer, rdp_string_message(resp_seg).c_str());
		if ((numbytes_sent = sendto(sockfd,send_buffer, sizeof(send_buffer), 0,
       		  (sockaddr *) &rec_addr, rec_len)) == 1) {
        		close(sockfd);
                	error ("in sendto()");
        	}
		log_event('s', "ACK", resp_seg, sender_addr, rec_addr );
		state = WaitToSend;
		//debugprint("state change to SEND");
	} else{debugprint("HS2 failed");}
}
*/
int main(int argc, char **argv) {
	struct sockaddr_in sender_addr;
    	struct sockaddr_in recv_addr; 
	//struct rdp_connection sender;
	fd_set read_fds;
	fd_set reads_fds;
    	int result;
	
	//printf("nhsakrgbadkjrh\n");
    	// Ensure proper command line arguments.
    	if (argc < 6) {
       		printf("usage: %s [sender_ip] [sender_port] [receiver_ip] [receiver_port] [sender_file_name]\n", *argv);
        	exit(EXIT_FAILURE);
    	}
	
	char * sender_ip = argv[1];
	char * sender_port = argv[2];
	char * receiver_ip = argv[3];
	char * receiver_port = argv[4];
	char * sender_file_name = argv[5];

	//printf("------opening file-------\n");
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&sender_addr, sizeof(sender_addr));
    	if (sockfd < 0) {
		perror("error: on socket()\n");
        	exit(EXIT_FAILURE);
   	}
	socklen_t sendlen, reclen, sendlen1;
    	memset(&sender_addr, 0, sizeof(sender_addr));
	//bzero(&sender_addr,sizeof(sender_addr));
    	sender_addr.sin_family = AF_INET;
    	sender_addr.sin_addr.s_addr = inet_addr(sender_ip);
    	sender_addr.sin_port = htons(atoi(sender_port));
	sendlen = sizeof(sender_addr);

    	memset(&recv_addr, 0, sizeof(recv_addr));
	//bzero(&recv_addr,sizeof(recv_addr));
    	recv_addr.sin_family = AF_INET;
    	recv_addr.sin_addr.s_addr = inet_addr(receiver_ip);
    	recv_addr.sin_port = htons(atoi(receiver_port));
	reclen = sizeof(recv_addr);

	//printf("sender.addr: %s, sender.length:%d \n",  inet_ntoa(sender.sin_addr), sendlen1);

	//printf("%d \n",SYN);
	// bind socket
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &optval, sizeof(optval));
 	/*
	if (bind(sockfd,(struct sockaddr *)&sender_addr,sizeof(sender_addr)) < 0) {
		perror("error: on binding\n");
    		exit(EXIT_FAILURE);
	}*/

	sendSYN(sockfd, recv_addr, tmpBuffer, reclen);
	printf("Sender: attempting to SYN with receiver,curr state is Synchronizing\n");
	//timeval timeout;
	while(1){		
		FD_ZERO(&reads_fds);
		FD_SET(0, &read_fds);
		FD_SET(sockfd, &read_fds);
		if (select(sockfd+1, &read_fds, 0, 0, 0) == -1){
			perror("Failed to select!\n");
		}		
		
		//timeout.tv_sec = 5;
		//timeout.tv_usec = 0;

		if (FD_ISSET(sockfd, &read_fds)) {
			//printf("Received something------\n");
			if (recvfrom(sockfd, (void*)tmpBuffer, sizeof tmpBuffer, 0, (struct sockaddr*)&recv_addr, &reclen) < 0) {
				perror("sws: error on recvfrom()!\n");
				exit(EXIT_FAILURE);
     			}
			packet.type = *tmpBuffer;
			if (packet.type == ACK){
				printf("Received ACK from receiver side. Attempting to send packet data \n");
				memset(tmpBuffer, 0, sizeof(tmpBuffer)*sizeof(char));			
				openFile(sender_file_name, buffer);
				sendToReceiver(sockfd, recv_addr, buffer, reclen);
				
			}else if(packet.type == DAT){
				packet.type = FIN;
				tmpBuffer[0] = packet.type;
        			if ((sendto(sockfd, tmpBuffer, strlen(tmpBuffer), 0,
	 (struct sockaddr*)&recv_addr, reclen)) < 0 ) {
					perror("error: send message failed\n");
					exit(EXIT_FAILURE);
				}
				printf("recv_addr: %s, content: %s\n",  inet_ntoa(recv_addr.sin_addr), tmpBuffer);
				memset(tmpBuffer, 0, sizeof(tmpBuffer)*sizeof(char));
				break;
			}
		}
	}
	printMessage();	

	
}
