/* Socket
 *
 * Author: Andy Pabst
 * Date 3/1/24 */

#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define BUF_SIZE 500

bool caught_a_signal = false;
bool caught_sigint = false;
bool caught_sigterm = false;

bool should_continue = true;



static void signal_handler(int signal_number) {
    caught_a_signal = true;
    if (signal_number == SIGINT) {
        caught_sigint = true;
    }
    if (signal_number == SIGTERM) {
        caught_sigterm = true;
    }
}

int main (int argc, char*argv[]) {

    openlog(NULL, 0, LOG_USER);    
    
    printf("Starting.");

    // set up signal handling
    struct sigaction new_action;
    bool success = true;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    int sa = sigaction(SIGINT, &new_action, NULL);
    if (sa != 0) {
        syslog(LOG_ERR, "Failed to register for SIGINT. Error: %d", sa);
    }
    sa = sigaction(SIGTERM, &new_action, NULL);
    if (sa != 0) {
        syslog(LOG_ERR, "Failed to register for SIGTERM. Error: %d", sa);
    }

    printf("Signal handling set up.\n");

    struct addrinfo hints;
    struct addrinfo *serverinfo;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    
    const char *port = "9000";

    // open stream socket
    int r = getaddrinfo(NULL, port, &hints, &serverinfo);
    if (r != 0) {
        syslog(LOG_ERR, "getaddrinfo encountered an error: %d", r);
        printf("getaddrinfo encountered an error: %d\n", r);
	return -1;
    }

    int sfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (sfd == -1) {
        syslog(LOG_ERR, "Socket could not be created. Error: %d", errno);
        printf("Socket could not be created. Error: %d\n", errno);
        return -1;
    }

    r = bind(sfd, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if (r != 0) {
        syslog(LOG_ERR, "Socket failed to bind. Error: %d", errno);
        printf("Socket failed to bind. Error: %d\n", errno);
	return -1;
    }
    
    while(!caught_a_signal && should_continue) {
	//printf("Entering while loop.");
	// listen for and accept connection
	r = listen(sfd, 1);
	if (r != 0) {
	    syslog(LOG_ERR, "Error while listening on socket: %d", errno);
	    printf("Error while listening on socket: %d\n", errno);
	}
	struct sockaddr peer;
	socklen_t peer_addr_size;
	int pfd = accept(sfd, &peer, &peer_addr_size); 
	if (pfd == -1) {
	    syslog(LOG_ERR, "Error while acceptin connection: %d", errno);
	    printf("Error while acceptin connection: %d\n", errno);
	}
	syslog(LOG_INFO, "Accepted connection from %s", peer.sa_data);
	printf("Accepted connection from %s\n", peer.sa_data);

        // recieve data
	int output_file = open("/var/tmp/aesdsocketdata", O_RDWR | O_CREAT | O_APPEND, 0666);
        if (output_file < 0) {
	    syslog(LOG_ERR, "Could not open file. Error: %d", errno);
	    printf("Could not open file. Error: %d\n", errno);
	    break;
	}
	char buf[BUF_SIZE];
        ssize_t nread;
	char test[] = "123abc";

	do {
            nread = recv(sfd, buf, BUF_SIZE, 0);
	    if (nread < 0) {
	        syslog(LOG_ERR, "Could not recieve data. Error: %d", errno);
		printf("Could not recieve data. Error: %d\n", errno);
		should_continue = false;
	        break;
	    }
	    printf("Recieved %ld data.\n", nread);
            //ssize_t nr = write(output_file, buf, BUF_SIZE);
            ssize_t nr = write(output_file, test, 6);
	    if (nr < 0) {
		printf("Something went wrong: %i\n", errno);
		should_continue = false;
		break;
	    }
	    printf("Written %ld data.\n", nr);
	} while (nread == BUF_SIZE);

        syslog(LOG_INFO, "Closing connection at %s", peer.sa_data);
	close(sfd);
    }
 
               
    if (caught_sigint || caught_sigterm) {
        close(sfd);
	freeaddrinfo(serverinfo);
	//TODO delete file
        syslog(LOG_INFO, "Caught signal, exiting.");
	exit(0);	
    }
}
