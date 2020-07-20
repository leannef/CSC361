#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string>
#include <sys/select.h>
#include "rdp_header.h"

		//CP: https://github.com/ColtonPhillips/csc361/tree/master/p2
		//Ye:https://github.com/Aceyee/CSC361/blob/master/A2%20-%20Reliable%20Datagram%20Protocol/rdps.c
		//bb: https://github.com/bbinn/csc361_a2
		//https://github.com/leonsenft/csc361/blob/master/assignment2/rdp.c#L46

char sendBuffer[1024], recBuffer[1024],tmpbuffer[1024];
//RdpState = state;

void printMessage(){
	printf("total data bytes received: \n");
	printf("unique data bytes received: \n");
	printf("total data packets received: \n");
	printf("unique data packets received: \n");
	printf("SYN packets received: \n");
	printf("FIN packets received: \n");
	printf("RST packets received: 0\n");
	printf("ACK packets sent: \n" );
	printf("RST packets sent: 0\n");
	printf("total time duration (second): \n");
}

/*
//in state of received SYN from sender
void handleHandShake1() {
	if ((numbytes_rec = recvfrom(sockfd, rec_buffer, 
	 sizeof(rec_buffer),0,
	 (sockaddr *) &sender_addr, &sender_len)) == -1) {
		error("error: on recvfrom()!");
	}
	rdp_segment rec_seg = generate_segment(rec_buffer);
	if (rec_seg.SYN != true) {debugprint("HS1 failed"); } 
	else { 
		//debugprint("HandShake 1 received");
		log_event('r', "SYN", rec_seg,sender_addr,rec_addr);
		//print_rdp_message(rec_seg);
		
		// 3W-HandShake Part 2
		rdp_segment resp_seg = rec_seg;
		resp_seg.ackno = rec_seg.seqno+1;
		resp_seg.seqno = random_seq();
		isn = resp_seg.seqno;
		rdp_payload_set(resp_seg,"HSP2");
		strcpy(send_buffer, rdp_string_message(resp_seg).c_str());
		if ((numbytes_sent = sendto(sockfd,send_buffer, sizeof(send_buffer), 0,
		 (sockaddr *) &sender_addr, sender_len)) == 1) {
			close(sockfd);
			error ("in sendto()");
		}
		log_event('s', "SYN+ACK", resp_seg, sender_addr, rec_addr);
		state = HandShake3;
		//debugprint("state change to HandShake3");
	}	
}
*/
int main(int argc, char * argv[]) {	
	struct sockaddr_in sender_addr;
    	struct sockaddr_in recv_addr;
	//struct rdp_connection sender;
	int result; 
	fd_set read_fds;
	fd_set reads_fds;
	if (argc != 4) {
        	printf("usage: %s [receiver_ip] [receiver_port] [filename]\n", *argv);
        	exit(EXIT_FAILURE);
	}
	char * receiver_ip = argv[1];
	char * receiver_port = argv[2];
	char * filename = argv[3];

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	//bzero(&sender_addr, sizeof(sender_addr));
    	if (sockfd < 0) {
		perror("error: on socket()\n");
        	exit(EXIT_FAILURE);
   	}

	socklen_t sendlen, reclen;
    	memset(&sender_addr, 0, sizeof(sender_addr));
    	memset(&recv_addr, 0, sizeof(recv_addr));
    	recv_addr.sin_family = AF_INET;
    	recv_addr.sin_addr.s_addr = inet_addr(receiver_ip);
    	recv_addr.sin_port = htons(atoi(receiver_port));
	reclen = sizeof(reclen);

	socklen_t senderlen = sizeof(sender_addr);
	//printf("sender.addr: %s, sender.length:%u \n",  inet_ntoa(sender.sin_addr), senderlen);
	// bind socket
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &optval, sizeof(optval));
 	
	if (bind(sockfd,(sockaddr *) &recv_addr, sizeof(recv_addr)) < 0) {
		perror("receiver: error: on binding\n");
    		exit(EXIT_FAILURE);
	}
	
	
	//state = HandShake1;
	while(1){
		FD_ZERO(&reads_fds);
		FD_SET(0, &read_fds);
		FD_SET(sockfd, &read_fds);
		if (select(sockfd+1, &read_fds, 0, 0, 0) == -1){
			perror("Failed to select!\n");
		}				
		if (FD_ISSET(sockfd, &read_fds)){
			if (recvfrom(sockfd, (void*)recBuffer, sizeof recBuffer, 0, (struct sockaddr*)&sender_addr, &senderlen)< 0) {
				perror("receiver: error on recvfrom()!\n");
				exit(EXIT_FAILURE);
     			}			
			//printf("size : %d", sizeof recBuffer);
			strcpy(tmpbuffer, recBuffer);
			packet.type = tmpbuffer[0];
			
			if(packet.type == SYN){		
				printf("Received SYN from Sender. \n");				
				memset(recBuffer, 0, sizeof(recBuffer)*sizeof(char));
				packet.type =ACK;
				recBuffer[0] = packet.type;			
				if(sendto(sockfd, recBuffer, strlen(recBuffer), 0,(struct sockaddr*)&sender_addr, senderlen) < 0){
					perror("receiver: error on sendto()ACK!\n");
					exit(EXIT_FAILURE);
			}
				printf("Receiver send ACK success. \n");										
				if (result < 0) {
					perror("receiver: error on recvfrom()!\n");
					exit(EXIT_FAILURE);
     				}
				memset(recBuffer, 0, sizeof(recBuffer)*sizeof(char));			
				memset(tmpbuffer, 0, sizeof(tmpbuffer)*sizeof(char));			
					
			}else if(packet.type == DAT){	
					
				printf("Received packet data from Sender.Send ACK to sender Waiting for finish. \n");
				printf("%s\n", recBuffer);						
				memset(recBuffer, 0, sizeof(recBuffer)*sizeof(char));
				packet.type = DAT;
				recBuffer[0] = packet.type;			
				if(sendto(sockfd, recBuffer, strlen(recBuffer), 0,(struct sockaddr*)&sender_addr, senderlen) < 0){
					perror("receiver: error on sendto()ACK!\n");
					exit(EXIT_FAILURE);
				}
									
				if (result < 0) {
					perror("receiver: error on recvfrom()!\n");
					exit(EXIT_FAILURE);
     				}
				memset(recBuffer, 0, sizeof(recBuffer)*sizeof(char));					
				memset(tmpbuffer, 0, sizeof(tmpbuffer)*sizeof(char));						
			}else if(packet.type == FIN){	
					
				printf("Received FIN signal from Sender. \n");
						
				break;			
			}
	
			}
			
		}
	
	printMessage();	

}
