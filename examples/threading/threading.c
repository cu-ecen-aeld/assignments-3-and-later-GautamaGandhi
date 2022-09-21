#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    int ret = usleep(thread_func_args->wait_to_obtain_ms * 1000);

    if (ret == -1) {
        ERROR_LOG("Error in usleep with code: %d", errno);
        thread_func_args->thread_complete_success = false;
        thread_func_args->error_status = 1;
        return thread_param;
    } 

    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("Error in mutex lock with code: %d", errno);
        thread_func_args->thread_complete_success = false;
        thread_func_args->error_status = 1;
        return thread_param;
    }
    ret = usleep(thread_func_args->wait_to_release_ms * 1000);
    if (ret == -1) {
        ERROR_LOG("Error in usleep with code: %d", errno);
        thread_func_args->thread_complete_success = false;
        thread_func_args->error_status = 1;
        return thread_param;
    } 
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("Error in mutex unlock with code: %d", errno);
        thread_func_args->thread_complete_success = false;
        thread_func_args->error_status = 1;
        return thread_param;
    }
    
    if (thread_func_args->error_status == 0)
        thread_func_args->thread_complete_success = true;
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    // Dynamic allocation for thread_data structure and initialisation using function parameters
    struct thread_data *thread_param = (struct thread_data *)malloc(sizeof(struct thread_data));
    thread_param->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_param->wait_to_release_ms = wait_to_release_ms;
    thread_param->mutex = mutex;
    thread_param->thread = *thread;
    thread_param->error_status = 0;

    // Creating a thread to run threadfunc with parameters thread_param
    int ret = pthread_create(thread, NULL, threadfunc, thread_param);
    if (ret != 0) {
        ERROR_LOG("Error in pthread_create with err number: %d", errno);
    } else if (ret == 0) {
        DEBUG_LOG("Success in creating thread!");
        return true;
    }

    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    return false;
}

