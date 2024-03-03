/* Socket Header
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
#include <arpa/inet.h>

#define PORT "9000"
#define OUTPUT_FILE "/var/tmp/aesdsocketdata"
#define BUF_SIZE 500

static void signal_handler(int signal_number);
void set_signal_handling();
int open_socket();
void recieve_socket_data(int sockfd);
void return_socket_data(int sockfd);
