#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFERSIZE 256
#define MSGSIZE 65536

// host = fring.ccs.neu.edu
// reference: Beej's Guide to Network Programming: Using Internet Sockets

int main(int argc, char** argv) {

	int verbose = 0;

	int port = 27993;
	int ssl = 0;
	char host[BUFFERSIZE];
	char neuid[BUFFERSIZE];
	char ip[INET_ADDRSTRLEN];

	switch(argc) {
		// Just Host and NEU ID
		case 3:
			strncpy(host, argv[1], sizeof(host));
			strncpy(neuid, argv[2], sizeof(neuid));
			break;
		// SSL
		case 4:
			ssl = 1;
			strncpy(host, argv[2], sizeof(host));
			strncpy(neuid, argv[3], sizeof(neuid));
			break;
		// Port
		case 5:
			port = atoi(argv[2]);
			strncpy(host, argv[3], sizeof(host));
			strncpy(neuid, argv[4], sizeof(neuid));
			break;
		// SSL & Port
		case 6:
			port = atoi(argv[2]);
			ssl = 1;
			strncpy(host, argv[4], sizeof(host));
			strncpy(neuid, argv[5], sizeof(neuid));
			break;
		default:
			fprintf(stderr, "Input error\n./client <-p port> <-s> [hostname] [NEU ID]\n");
			return -1;
	}

	if (verbose)
		printf("HOST = %s\nNEUID = %s\nSSL = %d\nPORT=%d\n", host, neuid, ssl, port);

	/*********************************************************************************/
	/* Get IP */
	/*********************************************************************************/

	struct addrinfo hints, *res, *p;
	int status;
	void *addr;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;


	if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "%s %d: getaddrinfo: %s\n", __func__, __LINE__, gai_strerror(status));
		freeaddrinfo(res);
		return -1;
	}

	// get the pointer to the address itself,
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
	addr = &(ipv4->sin_addr);

	inet_ntop(res->ai_family, addr, ip, sizeof(ip));
	if (verbose)
		printf("%s %d: IP = %s\n", __func__, __LINE__, ip);

	/*********************************************************************************/
	/* Open Socket & Connect */
	/*********************************************************************************/

	
	int sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if (sockfd < 0) {
		fprintf(stderr, "%s %d: Socket creation error\n", __func__, __LINE__);
		freeaddrinfo(res);
		return -1;
	}

	// Set Port
	struct sockaddr_in *serv_addr = (struct sockaddr_in *)res->ai_addr;
	serv_addr->sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr->sin_addr) <= 0) {
		fprintf(stderr, "%s %d: Invalid address / Address not supported\n",  __func__, __LINE__);
		freeaddrinfo(res);
		close(sockfd);
		return -1; 
    } 

	if (connect(sockfd, (struct sockaddr *)serv_addr, res->ai_addrlen) < 0) {
		fprintf(stderr, "%s %d: Connection error\n",  __func__, __LINE__); 
		freeaddrinfo(res);
		close(sockfd);
		return -1;
    }

	freeaddrinfo(res);

	/*********************************************************************************/

	// HELLO
	// cs3700fall2019 HELLO [your NEU ID]\n

	char hello[BUFFERSIZE + 100];
	sprintf(hello, "cs3700fall2019 HELLO %s\n", neuid);

	if (verbose)
		printf("%s %d: Sending Hello Message = '%s'\n", __func__, __LINE__, hello);

	if (send(sockfd, hello, strlen(hello), 0) < 0) {
		fprintf(stderr, "%s %d: Sending error\n",  __func__, __LINE__);
		close(sockfd);
		return -1;
	}

	/*********************************************************************************/
	/* Find / Count */
	// cs3700fall2019 COUNT [the count of the given symbol in the given string]\n

	int keepFinding = 1;
	int findMessages = 0;


	// Loop through to continue to respond to FIND messages
	while (keepFinding == 1) {

		findMessages++;

		char countReturn[MSGSIZE];
		int countStatus;

		memset(countReturn, 0, sizeof(countReturn));

		if (countStatus = recv(sockfd, countReturn, sizeof(countReturn), 0) < 0) {
			fprintf(stderr, "%s %d: recv: error\n",  __func__, __LINE__);
			close(sockfd);
			return -1;
		}
		else if (countStatus == 0) {
			if (verbose)
				printf("%s %d: Server closed connection\n", __func__, __LINE__);
		}

		// TODO
		if (verbose) {
			printf("%s %d: Find response #%d\n", __func__, __LINE__, findMessages);
			printf("%s %d: Message start = \n%s\n", __func__, __LINE__, countReturn);
			printf("%s %d: Last char is '%c'\n", __func__, __LINE__, countReturn[strlen(countReturn) - 1]);
		}

		// If message was not all recieved
		int repeat = 0;

		if (countReturn[strlen(countReturn) - 1] != '\0' && countReturn[strlen(countReturn) - 1] != '\n') {
			repeat = 1;
		}

		while (repeat == 1) {

			if (verbose)
				printf("%s %d: Repeating for rest of message #%d\n", __func__, __LINE__, findMessages);

			char tempCountReturn[MSGSIZE];
			int tempCountStatus;

			memset(tempCountReturn, 0, sizeof(tempCountReturn));

			if (tempCountStatus = recv(sockfd, tempCountReturn, sizeof(tempCountReturn), 0) < 0) {
				fprintf(stderr, "%s %d: recv: error\n", __func__, __LINE__);
				close(sockfd);
				return -1;
			}
			else if (tempCountStatus == 0) {
				if (verbose)
					printf("%s %d: Server closed connection\n", __func__, __LINE__);
			}

			if (verbose)
				printf("%s %d: Recieved remaing part of message = \n%s\n", __func__, __LINE__, tempCountReturn);

			strcat(countReturn, tempCountReturn);
			if (tempCountReturn[strlen(tempCountReturn) - 1] == '\0' || tempCountReturn[strlen(tempCountReturn) - 1] == '\n') {
				if (verbose)
					printf("%s %d: Found end of message #%d\n", __func__, __LINE__, findMessages);
				repeat = 0;
				break;
			}
			if (verbose)
				printf("%s %d: Repeating for rest of message #%d\n", __func__, __LINE__, findMessages);
		}

		// Check if new message is FIND again
		if (strstr(countReturn, "FIND") == NULL) {
			if (verbose)
				printf("%s %d: Find messages end\n", __func__, __LINE__);
			if (strstr(countReturn, "BYE") != NULL) {
				printf("%s", strstr(countReturn, "BYE") + 4);
			}
			keepFinding = 0;
			break;
		}

		// Count ASCI Char

		char asci = countReturn[strstr(countReturn, "FIND") - countReturn + 5];
		char* randomstring = strstr(countReturn, "FIND") + 7;
		if (verbose) {
			printf("%s %d: Found ASCI Char '%c' for #%d\n", __func__, __LINE__, asci, findMessages);
			//printf("%s %d: Random String is %s\n", __func__, __LINE__, randomstring);
		}

		int count = 0;
		for (int ii = 0; ii < strlen(randomstring); ii++) {
			if (randomstring[ii] == asci) {
				count++;
			}
		}

		char countmsg[100];
		sprintf(countmsg, "cs3700fall2019 COUNT %d\n", count);
		if (verbose)
			printf("%s %d: Count MSG = '%s'\n", __func__, __LINE__, countmsg);

		if (send(sockfd, countmsg, strlen(countmsg), 0) < 0) {
			fprintf(stderr, "%s %d: Sending error\n", __func__, __LINE__);
			close(sockfd);
			return -1;
		}
	}

	/*********************************************************************************/

	close(sockfd);

	return 0;
}
