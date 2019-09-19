#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFERSIZE 256

//./client <-p port> <-s> [hostname] [NEU ID]

int main(int argc, char** argv) {

	int port = 27993;
	int ssl = 0;
	char host[BUFFERSIZE];
	char neuid[BUFFERSIZE];

	switch(argc) {
		case 3:
			strncpy(host, argv[1], BUFFERSIZE);
			strncpy(neuid, argv[2], BUFFERSIZE);
			break;
		case 4:
			ssl = 1;
			strncpy(host, argv[2], BUFFERSIZE);
			strncpy(neuid, argv[3], BUFFERSIZE);
			break;
		case 5:
			port = atoi(argv[2]);
			strncpy(host, argv[3], sizeof(argv[3]));
			strncpy(neuid, argv[4], BUFFERSIZE);
			break;
		case 6:
			port = atoi(argv[2]);
			ssl = 1;
			strncpy(host, argv[4], sizeof(argv[4]));
			strncpy(neuid, argv[5], BUFFERSIZE);
			break;
		default:
			printf("Input error\n./client <-p port> <-s> [hostname] [NEU ID]\n");
			return -1;
	}

	printf("HOST = %s\nNEUID = %s\nSSL = %d\nPORT=%d\n", host, neuid, ssl, port);

	struct hostent *he;
	struct in_addr **addr_list;
	char ip[256];

	if ((he = gethostbyname(host)) == NULL) 
	{
		fprintf(stderr, "gethostbyname error\n");
		return -1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	strncpy(ip, inet_ntoa(*addr_list[0]), 256);

	printf("ip = %s\n", ip);
	

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Socket creation error");
		return -1;
	}
	
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		printf("Invalid address / Address not supported \n");
		return -1; 
    } 

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Connection error\n"); 
		return -1;
    } 

	return 0;
}

int sendHello() {
	return 0;
}