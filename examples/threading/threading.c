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
    struct thread_data* data = (struct thread_data *) thread_param;
    DEBUG_LOG("Thread started. Going to wait %d ms before obtaining the mutex.", data->obtain_wait);
    
    usleep(data->obtain_wait*1000);
    int rc = pthread_mutex_lock(data->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread mutex lock failed. Code: %d", rc);
	data->thread_complete_success = false;
    }
    
    DEBUG_LOG("Mutex obtained. Going to wait %d ms before releasing.", data->release_wait);

    usleep(data->release_wait*1000);
    rc = pthread_mutex_unlock(data->mutex);
    if (rc != 0) {
        ERROR_LOG("pthread mutex unlock failed. Code: %d", rc);
	data->thread_complete_success = false;
    }

    DEBUG_LOG("Mutex unlocked. Ending thread.")

    if (rc == 0) {
        data->thread_complete_success = true;
    }	
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
   
    struct thread_data *data = (thread_data*)malloc(sizeof(thread_data));
    data->obtain_wait = wait_to_obtain_ms;
    data->release_wait = wait_to_release_ms;

    data->thread_complete_success = false;
    data->mutex = mutex;
    
    DEBUG_LOG("Creating Thread");
    rc = pthread_create(thread, NULL, threadfunc, data);
    if (rc != 0) {
	ERROR_LOG("Thread could not be created. Code: %d", rc);
    }
    
    DEBUG_LOG("Thread started");
    return true;
}

