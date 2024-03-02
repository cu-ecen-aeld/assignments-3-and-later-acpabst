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

#include <stdio.h>

int main (int argc, char*argv[]) {

    openlog(NULL, 0, LOG_USER);    

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
	return -1;
    }

    int sfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (sfd == -1) {
        syslog(LOG_ERR, "Socket could not be created. Error: %d", errno);
        return -1;
    }

    r = bind(sfd, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if (r != 0) {
        syslog(LOG_ERR, "Socket failed to bind. Error: %d", errno);
	return -1;
    }

    syslog(LOG_DEBUG, "Made it this far!");
    
    // listen for and accept connection
    r = listen(sfd, 1);
    if (r != 0) {
        syslog(LOG_ERR, "Error while listening on socket: %d", errno);
    }
    struct sockaddr peer;
    socklen_t peer_addr_size;
    int pfd = accept(sfd, &peer, &peer_addr_size); 
    if (pfd == -1) {
        syslog(LOG_ERR, "Error while acceptin connection: %d", errno);
    }
    syslog(LOG_INFO, "Accepted connection from %s", peer.sa_data);
 
 
 
    freeaddrinfo(serverinfo);

}
