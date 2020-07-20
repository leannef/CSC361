#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>

/*
CSC361 P1 SWS
Leanne Feng
V00825004
*/
 
char buffer[1024],pathname[512],command[4][1024];
char sendBuf[1024*20];
char *path;
long int file_size;
size_t bytes_read;
int i;

int printtime(){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf ("Feb %d %d:%d:%d ", tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}


//separate string whenever there is a " "
int separate(char mystring[]){
	i = 0;
	char *partial;
	partial = strtok (mystring," ");
	while (partial != NULL){
		strcpy(command[i], partial);
	 	partial = strtok (NULL, " ");
		i++;
	}
	//printf("%s\n", command[0]);
	//printf("%s\n", command[1]);
	//printf("%s\n", command[2]);

	if(strlen(command[i-1])>8){
			command[i-1][8] = '\0';
	}

}

//returns the string it recieved, only with all the letters in uppercase.
char *uppercase(char *str){
	unsigned char *tmp = (unsigned char *)str;
	while (*tmp){
		*tmp = toupper(*tmp);
		tmp++;
	}
	return str;
}

bool compare(char request[]){
	bool flag = true;
	char tempString[512];
	strcpy(tempString, request);
	separate(tempString); 
	bool check = false;	
	if (command[1][0] == '/'){
		check = true;
	} 
	if (!check){
		flag = false;
	}
	if (strcmp(uppercase(command[0]), "GET") != 0){
		flag = false;
	}

	if (strcmp(command[2], "HTTP/1.0") != 0){
		flag = false;
	}
	return flag;
}

//Checks if the file exsits
//reference:https://github.com/UltravioletVoodoo/Csc361_P1/blob/master/sws.c
int checkExist(char * filename){
	struct stat mybuffer;
	return (stat (filename, &mybuffer) == 0);
}

//if returns true the content of url is in buffer, else request case if 404
bool findfile(){
	FILE *fp;
	int c;
	strcpy(path, pathname);
	if ((int)strlen(command[1]) == 1){
		//printf("-------in if loop findfile---------\n");
		strcat(path, "/index.html");		
	}else{
		strcat(path, command[1]);
	}
	if (checkExist(path)){
		//printf("opening file\n");
		fp = fopen (path, "r");
	}else{	
		//printf("im here\n");
		return false;
	}

	//from lab5 code
	fseek (fp, 0L, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	bytes_read = fread(buffer, sizeof(char), file_size, fp);	
	fclose(fp);
	return true;
}

// return cases:  200, 400, or 404 
int requestCase(char request[]){	
	if (strcmp(request, "") == 0){
		return 400;
	}	
	bool valid = compare(request);
	if (valid){
		if (findfile()){
			return 200;
		}
		//printf("findfile returned false\n");
		return 404;
	}
	return 400;
}

// some code draws from lab code here
int main(int argc, char **argv){
        char *portno;
	struct sockaddr_in sa; 
	ssize_t recsize;
	fd_set read_fds;
	fd_set reads_fds;
	socklen_t fromlen;
	int result, ret, sendLen;

        char tmpbuffer[1024] ;

	if (argc != 3){
		printf("Incorrect number of arguments.\n");
		return EXIT_FAILURE;
	}
	portno = argv[1];
	path = argv[2];
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);	
	if ((socket(AF_INET, SOCK_STREAM, 0)) == -1){ 
		perror("sws: error on socket()");
	  	return -1;
     	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(atoi(portno));
	fromlen = sizeof(sa);
	
	int optval =1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
		close(sock);
		perror("setsockopt error!\n");
		exit(EXIT_FAILURE);	
	
	}

	if (bind(sock, (struct sockaddr *)&sa, sizeof sa) == -1) {
        	close(sock);
		perror("sws: error on binding!");
	  	return -1;
	}
	//printf("Bind Success.\n");

	printf("sws is running on UDP port %s and serving %s\n", portno, path);
	printf("press 'q' to quit ...\n");

	strcat(pathname, path);

	while (true){
		FD_ZERO(&reads_fds);
		FD_SET(0, &read_fds);
		FD_SET(sock, &read_fds);
	
		if (select(sock+1, &read_fds, 0, 0, 0) == -1){
			perror("Failed to select!");
			close(sock);
		}
		if (FD_ISSET(0, &read_fds)){
			fgets(buffer, (sizeof buffer)-1, stdin);
			if (buffer[0] == 'q'){
				close(sock);
				exit(EXIT_SUCCESS);
			}else{
				printf("Unrecognized Command.\n");
			}
		}
		if (FD_ISSET(sock, &read_fds)){			
			if (recvfrom(sock, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen) < 0) {
				perror("sws: error on recvfrom()!\n");
				exit(EXIT_FAILURE);
     			}		

			result = requestCase(buffer);	
			printtime();

			if (result == 200){
				//output 200 ok in server, output 200 ok tempbuffer and content of url buffer.
				memset(tmpbuffer, 0, sizeof(tmpbuffer)*sizeof(char));
				strcpy(tmpbuffer, "HTTP/1.0 200 ok\n");
				sendto(sock, tmpbuffer, strlen(tmpbuffer), 0,(struct sockaddr*)&sa, sizeof sa);
			
				printf("%s:%d %s %s %s; %s 200 OK; %s\n",inet_ntoa(sa.sin_addr),sa.sin_port, command[0], command[1], command[2], command[2], path);
				//sending single message
				if (bytes_read < sizeof(buffer)){
					sendto (sock, (void*)buffer, (size_t)strlen(buffer), 0, (struct sockaddr*)&sa, sizeof(sa));
				}else{
					//sengding long. multi-packet messages
					int index = 0;
					while (index < bytes_read){
						sendto(sock, &buffer[index], (size_t)strlen(buffer), 0, (struct sockaddr*)&sa, sizeof(sa));
						index += (size_t)strlen(buffer);
					}
				}	
				memset(buffer, 0, sizeof(buffer)*sizeof(char));
			}else if(result == 400){
				//output 400 in both server and client
				printf("%s:%d %s %s %s; %s 400 Bad Request; %s\n", inet_ntoa(sa.sin_addr),sa.sin_port,command[0], command[1], command[2], command[2], pathname);
				strcpy(buffer, "HTTP/1.0 400 Bad Request\n");
				sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr*)&sa, sizeof sa);
				memset(buffer, 0, sizeof(buffer)*sizeof(char));
			}else if(result == 404){   
				//output 400 in both server and client  
				printf("%s:%d %s %s %s; %s 404 Not Found; %s\n", inet_ntoa(sa.sin_addr),sa.sin_port,command[0], command[1], command[2], command[2], path);
				strcpy(buffer, "HTTP/1.0 404 Not Found\n");
				sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr*)&sa, sizeof sa);
				memset(buffer, 0, sizeof(buffer)*sizeof(char));
				
			}
		}	
		memset(buffer, 0, sizeof(buffer)*sizeof(char));	
	}
}
