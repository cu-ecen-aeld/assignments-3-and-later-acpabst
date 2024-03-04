/* Socket
 *
 * Author: Andy Pabst
 * Date 3/1/24 */

#include "aesdsocket.h"

#define BUF_SIZE 500

bool caught_a_signal = false;
bool caught_sigint = false;
bool caught_sigterm = false;

bool should_continue = true;



static void signal_handler(int signal_number) {
    caught_a_signal = true;
    should_continue = false;
    if (signal_number == SIGINT) {
        caught_sigint = true;
    }
    if (signal_number == SIGTERM) {
        caught_sigterm = true;
    }
}

void set_signal_handling() {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler; 
    int sa = sigaction(SIGINT, &new_action, NULL);
    if (sa != 0) {
        syslog(LOG_ERR, "Failed to register for SIGINT. Error: %d", sa);
        exit(-1);
    }
    sa = sigaction(SIGTERM, &new_action, NULL);
    if (sa != 0) {
        syslog(LOG_ERR, "Failed to register for SIGTERM. Error: %d", sa);
        exit(-1);
    }

    syslog(LOG_INFO, "Signal Handling set up complete.");
    printf("Signal handling set up.\n");
}

int open_socket() {
    struct addrinfo hints;
    struct addrinfo *serverinfo;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;

    int r = getaddrinfo(NULL, PORT, &hints, &serverinfo);
    if (r != 0) {
        syslog(LOG_ERR, "getaddrinfo encountered an error: %d", r);
        exit(-1);
    }

    int sfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (sfd == -1) {
        syslog(LOG_ERR, "Socket could not be created. Error: %d", errno);
        exit(-1);
    }
    const int opt_yes = 1;
    r = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt_yes, sizeof(opt_yes));
    if (r == -1) {
        syslog(LOG_ERR, "Socket options could not be set. Error: %d", errno);
        exit(-1);
    }

    r = bind(sfd, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if (r != 0) {
        syslog(LOG_ERR, "Socket failed to bind. Error: %d", errno);
        exit(-1);
    }
    freeaddrinfo(serverinfo);
    return sfd;
}

void recieve_socket_data(int sockfd) {
    char buf[BUF_SIZE+1];    
    ssize_t nrecv;
    ssize_t nwrit;
    char *ptr_null;
    
    int output_file = open(OUTPUT_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (output_file < 0) {
        syslog(LOG_ERR, "Could not open file. Error: %d", errno);
        should_continue = false;
	return;
    }

    do {
        nrecv = recv(sockfd, buf, BUF_SIZE, 0);
	buf[nrecv] = 0;
	if (nrecv < 0) {
            syslog(LOG_ERR, "Could not recieve data. Error: %d", errno);
            should_continue = false;
            return;
        }

        nwrit = write(output_file, buf, nrecv);
        if (nwrit < 0) {
            should_continue = false;
            return;
        }
	ptr_null = strchr(buf, '\n');
    } while (ptr_null == NULL);

    close(output_file);
    return;
}

void return_socket_data(int sockfd) {
    char buf[BUF_SIZE];    
    ssize_t nread;
    ssize_t nsend;

    int output_file = open(OUTPUT_FILE, O_RDONLY, 0666);
    if (output_file < 0) {
        syslog(LOG_ERR, "Could not open file. Error: %d", errno);
        should_continue = false;
	return;
    }

    do {    
        nread = read(output_file, buf, BUF_SIZE);
	if (nread < 0) {
            should_continue = false;
	    return;
        }
	nsend = send(sockfd, buf, nread, 0);
        if (nsend < 0) {
            syslog(LOG_ERR, "Could not send data. Error: %d", errno);
	    should_continue = false;
            return;
        }
    } while (nsend == BUF_SIZE);

    close(output_file);
    return;
}

void graceful_socket_shutdown(int sockfd) { 
    printf("Shutting down aesdsocket.");
    int r = shutdown(sockfd,SHUT_RDWR);
    if (r != 0) {
        syslog(LOG_ERR, "Socket shutdown failed. Error: %d", errno);
    }
    r = close(sockfd);
    if (r != 0) {
        syslog(LOG_ERR, "Socket close failed. Error: %d", errno);
    }
    remove(OUTPUT_FILE);
}

/* 
 * --------- MAIN --------- 
 */
int main (int argc, char*argv[]) {

    printf("Beginning aesdsocket!\n");
    openlog(NULL, 0, LOG_USER);    

    set_signal_handling();

    char buf[BUF_SIZE];
    size_t nread;
    
    int sfd = open_socket();

    if (argc >= 2 && !strcmp(argv[1], "-d")) {
	int r = fork();
	if (r < 0) {
            syslog(LOG_ERR, "Error creating deamon fork: %d", errno);
            printf("Error creating deamon fork: %d\n", errno);
	    exit(-1);
	} else if (r > 0) {
	    exit(0);
	}
    }
    
    do {	
        // listen for and accept connection
	int r = listen(sfd, 10);
	if (r != 0) {
	    syslog(LOG_ERR, "Error while listening on socket: %d", errno);
	    printf("Error while listening on socket: %d\n", errno);
	    graceful_socket_shutdown(sfd);
	    exit(-1);
	}

        struct sockaddr peer;
        socklen_t peer_addr_size = sizeof(peer);
        memset(&peer, 0, sizeof(peer));   //make sure struct is empty

        printf("Waiting for connection . . .\n");

	int pfd = accept(sfd, &peer, &peer_addr_size); 
	if (pfd == -1) {
	    if (errno == 4) {
                break;
	    }
	    syslog(LOG_ERR, "Error while accepting connection: %d", errno);
	    printf("Error while accepting connection: %d\n", errno);
	    graceful_socket_shutdown(sfd);
	    exit(-1);
	}
	char client[INET6_ADDRSTRLEN];
	inet_ntop(peer.sa_family, peer.sa_data, client, sizeof(client));
	syslog(LOG_INFO, "Accepted connection from %s", client);
	printf("Accepted connection from %s\n", client);

	recieve_socket_data(pfd);
	if (!should_continue) {
		break;
	}
	return_socket_data(pfd);
        syslog(LOG_INFO, "Closing connection at %s", client);
        printf("Closing connection at %s\n", client);
        shutdown(pfd,SHUT_RDWR);
        close(pfd);   
    } while(!caught_a_signal && should_continue);

    if (caught_sigint || caught_sigterm) {
	syslog(LOG_INFO, "Caught signal, exiting.");
	printf("Caught signal, exiting.\n");
        graceful_socket_shutdown(sfd);
        remove(OUTPUT_FILE);
	
    }
    exit(0);
}
