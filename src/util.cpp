#include <time.h>
#include <zconf.h>
#include <cstdlib>
#include <pthread.h>
#include <cstring>
#include <cerrno>
#include <cstdio>

#include "util.h"

/**
 * Delays a thread by `delay` milliseconds
 * @param delay The number of milliseconds to delay
 */
void delay(int delay) {
    struct timespec interval;
    interval.tv_sec = (long) delay / 1000;
    interval.tv_nsec = (long) ((delay % 1000) * 1000000);

    // pause the thread execution for a specified time
    if (nanosleep(&interval, NULL) < 0)
        printf("warning: delay: %s\n", strerror(errno));
}

/**
 * Initializes a mutex
 * @param mutex A pointer to the mutex variable
 */
void mutex_init(pthread_mutex_t *mutex) {
    int rval = pthread_mutex_init(mutex, NULL);
    if (rval) {
        fprintf(stderr, "mutex_init: %s\n", strerror(rval));
        exit(EXIT_FAILURE);
    }
}

/**
 * Locks a mutex
 * @param mutex A pointer to the mutex variable
 */
void mutex_lock(pthread_mutex_t *mutex) {
    int rval = pthread_mutex_lock(mutex);
    if (rval) {
        fprintf(stderr, "mutex_lock: %s\n", strerror(rval));
        exit(EXIT_FAILURE);
    }
}

/**
 * Unlocks a mutex
 * @param mutex A pointer to the mutex variable
 */
void mutex_unlock(pthread_mutex_t *mutex) {
    int rval = pthread_mutex_unlock(mutex);
    if (rval) {
        fprintf(stderr, "mutex_unlock: %s\n", strerror(rval));
        exit(EXIT_FAILURE);
    }
}

/**
 * Waits for the specified thread to terminate
 * @param pthread A pointer to the thread to wait for
 */
void do_pthread_join_with_error_check(pthread_t* pthread) {
    int rval = pthread_join(*pthread, NULL);
    if (rval) {
        fprintf(stderr, "\n** pthread_join: %s\n", strerror(rval));
        exit(EXIT_FAILURE);
    }
}

/**
 * Creates a new thread
 * @param start_function A pointer to the function that the new thread will execute
 * @param arg A pointer to the argument that will be passed to the new thread's start function
 */
void do_pthread_create_with_error_check(void *(*start_function)(void *), void *arg) {
    pthread_t threadID;
    int rval = pthread_create(&threadID, NULL, start_function, arg);

    // attr is NULL, so the thread is created with default attributes.
    if (rval) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rval));
        exit(EXIT_FAILURE);
    }
}
