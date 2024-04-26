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
#include <pthread.h>
#include <sys/time.h>

#include "queue.h"

#define USE_AESD_CHAR_DEVICE 1

#define PORT "9000"
#define BUF_SIZE (1024*30)

#ifdef USE_AESD_CHAR_DEVICE
#    define OUTPUT_FILE "/dev/aesdchar"
#else
#    define OUTPUT_FILE "/var/tmp/aesdsocketdata"
#endif
typedef struct thread_data {
    pthread_mutex_t *mutex;
    bool thread_complete_success;

    struct sockaddr peer;
    int pfd;
} thread_data;

// Adapted from https://github.com/stockrt/queue.h/blob/master/sample.c
SLIST_HEAD(slisthead, slist_data_s);
typedef struct slist_data_s slist_data_t;
struct slist_data_s {
    pthread_t thread_id;
    struct thread_data *tdata;
    SLIST_ENTRY(slist_data_s) entries;
};

static void signal_handler(int signal_number);
void set_signal_handling();
int open_socket();
int recieve_socket_data(int sockfd);
int return_socket_data(int sockfd);
