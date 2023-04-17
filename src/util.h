#ifndef UTIL_H
#define UTIL_H

#include <zconf.h> // for delay()

/**
 * Delays the current thread by `delay` milliseconds.
 * @param delay time to wait in milliseconds
 */
void delay(int delay);

/**
 * Initializes a mutex.
 * @param mutex pointer to a pthread_mutex_t struct
 */
void mutex_init(pthread_mutex_t* mutex);

/**
 * Locks a mutex.
 * @param mutex pointer to a pthread_mutex_t struct
 */
void mutex_lock(pthread_mutex_t* mutex);

/**
 * Unlocks a mutex.
 * @param mutex pointer to a pthread_mutex_t struct
 */
void mutex_unlock(pthread_mutex_t* mutex);

/**
 * Joins a thread and checks for errors.
 * @param pthread pointer to the thread ID
 */
void do_pthread_join_with_error_check(pthread_t* pthread);

/**
 * Creates a thread and checks for errors.
 * @param start_function pointer to the function to be executed by the new thread
 * @param arg pointer to the argument to pass to the start function
 */
void do_pthread_create_with_error_check(void *(*start_function)(void *), void *arg);

#endif //UTIL_H

