#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("\nthreading: " msg "\n\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("\nthreading ERROR: " msg "\n\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    DEBUG_LOG("Madde it here!");
   
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* data = (struct thread_data *) thread_param;
    DEBUG_LOG("intermediate step. going to wait %d ms", data->obtain_wait);
    
    usleep(data->obtain_wait*1000);
    DEBUG_LOG("Well, I think I waited");
    
    int rc = pthread_mutex_lock(data->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread mutex lock failed. Code: %d", rc);
	data->thread_complete_success = false;
    }

    usleep(data->release_wait*1000);
    
    rc = pthread_mutex_unlock(data->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread mutex unlock failed. Code: %d", rc);
	data->thread_complete_success = false;
    }

    if (rc == 0) {
        data->thread_complete_success = true;
    }	
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    struct thread_data *data = (thread_data*)malloc(sizeof(thread_data));
    data->obtain_wait = wait_to_obtain_ms;
    data->release_wait = wait_to_release_ms;

    int rc = 0;
    data->thread_complete_success = false;
    data->mutex = mutex;
    // Set up Mutex
    // int rc = pthread_mutex_init(&data->mutex,NULL);
    if (rc != 0) {
        ERROR_LOG("pthread mutex init failed. Code: %d", rc);
	return false;
    }
    //rc = pthread_mutex_lock(&data->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread mutex init failed. Code: %d", rc);
        return false;
    }
    // create thread
    rc = pthread_create(thread, NULL, threadfunc, data);
    if (rc != 0) {
	ERROR_LOG("thread could not be created. Code: %d", rc);
    }
    
    DEBUG_LOG("Thread started");
    return true;
}

