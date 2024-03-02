/* Socket
 *
 * Author: Andy Pabst
 * Date 3/1/24 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <syslog.h>

int main (int argc, char*argv[]) {

    struct addrinfo hints;
    struct addrinfo *serverinfo;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    

    // open stream socket at port 9000

    int r = getaddrinfo(NULL, '9000', &hints, &serverinfo);
    if (r != 0) {
        syslog(LOG_ERR, "getaddrinfo encountered an error: %d", r);
	return -1;
    }

    int fd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (fd == -1) {
        syslog(LOG_ERR, "Socket could not be created. Error: %d", errno);
        return -1;
    }

    r = bind(fd, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if (r != 0) {
        syslog(LOG_ERR, "Socket failed to bind. Error: %d", errno);
	return -1;
    }

    freeaddrindo(serverinfo);

}
