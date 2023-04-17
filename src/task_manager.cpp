// This code is for a task manager application that manages tasks with different resources.

#include "parsers.h"
#include "task_manager.h"
#include "util.h"
#include <string.h>
#include <sys/times.h>

// Global variables
std::map<std::string, int> resourceMap; // map of resources and their current availability
std::vector<TASK> taskList; // holds information about all tasks
pthread_t threads[NTASKS]; // holds thread IDs of worker threads

// Global variables for time tracking
uint ITERATIONS = 0; // number of iterations to run for each task
clock_t START = 0; // time when program started
clock_t END; // time when program ended
long _CLK_TCK = 0; // clock ticks per second

// Mutexes for thread synchronization
pthread_mutex_t threadMutex; // mutex to lock worker threads
pthread_mutex_t resourceMapMutex; // mutex to lock resourceMap
pthread_mutex_t monitorMutex; // mutex to lock monitor thread

/**
    Returns the time in milliseconds that have passed since reading the input file.
    */
float getTime() {
    tms tmsend;
    END = times(&tmsend);
    return (END - START) / (double) _CLK_TCK * 1000;
    }

/**
    Prints to the screen the status of tasks (WAITING, RUNNING, IDLE).
    */
void printMonitor() {
    // Initialize strings to hold names of tasks in each status
    std::string waitTasks;
    std::string runTasks;
    std::string idleTasks;

    // Iterate through taskList to add names of tasks to the appropriate string
    for (auto &task : taskList) {
            switch (task.status) {
                case WAIT:
                waitTasks.append(task.name);
                waitTasks.append(" ");
                break;
            case RUN:
                runTasks.append(task.name);
                runTasks.append(" ");
                break;
            default:
                idleTasks.append(task.name);
                idleTasks.append(" ");
        }
    }

    // Print the status of all tasks to the screen
    printf("Monitor: [WAIT] %s\n\t [RUN] %s\n\t [IDLE] %s\n\n", waitTasks.c_str(),
        runTasks.c_str(), idleTasks.c_str());
    }

/**

    Entry point for a monitor thread.
    Prints to the screen the STATUS of tasks
    every interval milliseconds.
    @param arg Pointer to the interval for the monitor thread to run
    @return Null pointer
    */
void *monitorThread(void *arg) {
    auto monitorTime = (long) arg;
    while (true) {
        delay(monitorTime); // wait for specified time
        mutex_lock(&monitorMutex); // acquire lock on monitorMutex
        printMonitor(); // print task status
        mutex_unlock(&monitorMutex); // release lock on monitorMutex
        }
    return nullptr;
    }

/**
    Returns whether all resources required by a task are available
    @param task Pointer to the task to check
    @return True if all required resources are available, false otherwise
    */
bool checkResourcesAvailable(TASK *task) {
    for (auto &resourceString : task->reqResources) {

    // Parse resource name and required number of units from the resource string
    char *saveptr;
    char resource[MAX_RESOURCE_LENGTH];

    strcpy(resource, resourceString.c_str());

    char *resName = strtok_r(resource, ":", &saveptr);
    int resNumber = atoi(strtok_r(nullptr, ":", &saveptr));

     if (resourceMap[resName] < resNumber) {
     return false;
    }
 }
return true;
}


// This function switches a task's status with the restriction that tasks cannot switch if the monitor is printing.
// It locks and unlocks the monitor mutex to ensure that task statuses will not change while the monitor thread is printing.
void switchStatus(TASK *task, STATUS status) {

    mutex_lock(&monitorMutex); //Lock the monitor mutex to prevent the monitor thread from printing while the task status is being updated.
    task->status = status; // Set the task's status to the new status.
    mutex_unlock(&monitorMutex);  //Unlock the monitor mutex.
}

/**
 * Locks the resource map, checks if resources are available for a resource,
 * and unlocks the map if they are not.
 */
void waitForResources(TASK *task) { // todo - fix does not work correctly
    switchStatus(task, WAIT);
    bool resAvailable = false;
    while (!resAvailable) {
        mutex_lock(&resourceMapMutex);
        resAvailable = checkResourcesAvailable(task);
        if (!resAvailable) {
            mutex_unlock(&resourceMapMutex);
            delay(20);
        }
    }
}


int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

/**
    This function adjusts the resources used by a task to be
    =(resourceMapValue (operation) taskResourceValue)
    in the resource map
    It requires wrapper functions to lock the map mutex.
    */
void adjustResources(TASK *task, int (*operation)(int, int)) {
    for (auto &resourceString : task->reqResources) { // Iterate through the task's required resources.
        char *saveptr;
        char resource[MAX_RESOURCE_LENGTH];

        strcpy(resource, resourceString.c_str()); // Copy the resource string to a local variable.
        char *resName = strtok_r(resource, ":", &saveptr); // Parse the resource name from the string.
        int resNumber = atoi(strtok_r(nullptr, ":", &saveptr)); // Parse the resource number from the string.
        int currentValue = resourceMap[resName]; // Get the current value of the resource from the resource map.

        resourceMap[resName] = operation(currentValue, resNumber); // Apply the arithmetic operation to the resource value in the map.

    }
}

/**
 * Removes the resources used by a task from the resource map
 * @param task The task whose resources should be procured
 */
void procureResources(TASK *task) {
    adjustResources(task, sub); // Adjust resources used by the task
    mutex_unlock(&resourceMapMutex); // Unlock the mutex for the resource map
}

/**
 * Adds the resources used by a task to the resource map
 * @param task The task whose resources should be released
 */
void releaseResources(TASK *task) {
    mutex_lock(&resourceMapMutex); // Lock the mutex for the resource map
    adjustResources(task, add); // Adjust resources used by the task
    mutex_unlock(&resourceMapMutex); // Unlock the mutex for the resource map
}

/**
 * Runs a single iteration of a task
 * @param task The task to run
 */
void runTaskIteration(TASK *task) {
    procureResources(task); // Procure the resources needed by the task (note: map mutex gets unlocked here)
    delay(task->busyTime); // Wait for the task's busy time
    task->totalBusyTime += task->busyTime; // Add the busy time to the task's total busy time
    releaseResources(task); // Release the resources used by the task
}

/**
 * Runs a single idle period for a task
 * @param task The task to run the idle period for
 */
void doTaskIdle(TASK *task) {
    delay(task->idleTime); // Wait for the task's idle time
    task->totalIdleTime += task->idleTime; // Add the idle time to the task's total idle time
}

/**
 * Runs a task ITERATIONS times
 * @param task The task to run
 */
void runTask(TASK *task) {
    clock_t iterStart, iterWait;
    struct tms tmsIterStart, tmsIterWait;
    uint iterCount = 0;

    while (iterCount != ITERATIONS) {
        switchStatus(task, WAIT); // Switch the task status to waiting
        iterStart = times(&tmsIterStart); // Record the start time of the iteration
        waitForResources(task); // Wait for resources to become available
        iterWait = times(&tmsIterWait); // Record the time the task waited for resources
        task->totalWaitTime += (iterWait - iterStart) * 1000 / _CLK_TCK; // Add the wait time to the task's total wait time

        switchStatus(task, RUN); // Switch the task status to running
        runTaskIteration(task); // Run a single iteration of the task

        switchStatus(task, IDLE); // Switch the task status to idle
        doTaskIdle(task); // Run a single idle period for the task

        task->timesExecuted += 1; // Increment the number of times the task has been executed
        iterCount++; // Increment the iteration count
        printf("task: %s (tid= %lu, iter= %d, time= %.0f msec) \n", task->name, pthread_self(),
               iterCount, getTime()); // Print out information about the task execution
    }
}

/**
 * Prints out final statistics for the system
 */
void printTerminationInfo() {
    std::string systemResources;
    std::string systemTasks;
    systemResources = getFormattedResourceInfo(); // Get formatted resource information
    systemTasks = getFormattedTaskInfo(); // Get formatted task information
    printf("System Resources:\n%s"
           "\n"
           "\n"
           "System Tasks: \n%s"
           "Running time= %.0f msec\n", systemResources.c_str(), systemTasks.c_str(), getTime());
}
/**

    Entry point for a TASK thread.
    Assigns the first unassigned thread to the current thread, then unlocks the
    thread mutex.
    @param arg the index of the current thread in the threads array
    */
void *task_start_routine(void *arg) {
    // Assign the first unassigned thread to the current thread
    threads[(long) arg] = pthread_self();
    for (auto &task : taskList) {
        if (task.assigned) {
            continue;
                            }
            task.assigned = true;
            // Unlock the thread mutex
            mutex_unlock(&threadMutex);
            // Run the task assigned to this thread
            runTask(&task);
            break;
        }
    pthread_exit(nullptr);
    }

/**

    Creates the monitor thread with the specified monitoring interval.
    @param time the monitoring interval in milliseconds
    */
    void createMonitorThread(long time) {
    do_pthread_create_with_error_check(&monitorThread, (void *) time);
    }

/**

    Creates all task threads and assigns tasks to them.
    */
    void createTaskThreads() {
    for (unsigned long i = 0; i < taskList.size(); i++) {
    mutex_lock(&threadMutex);
    do_pthread_create_with_error_check(task_start_routine, (void *) i);
    }
    }

/**

    Waits for all task threads to finish executing.
    */
    void waitForTaskTermination() {
    for (unsigned int i = 0; i < taskList.size(); i++) {
    do_pthread_join_with_error_check(&threads[i]);
    }
    }

/**
    Runs the multithreaded system simulation.
    Initializes mutexes, creates the monitor thread, creates task threads and waits
    for all tasks to finish executing before printing out the termination info.
    @param args the command line arguments for the simulation
    @return EXIT_SUCCESS if the simulation completes successfully
    */
    int run(CommandLineArguments args) {
    tms tmsstart;
    ITERATIONS = args.iterations;

    if ((_CLK_TCK = sysconf(_SC_CLK_TCK)) < 0) {
    printf("ERROR: getting sysconfig clock tick\n");
    exit(-1);
    }

    printf("Reading File...\n");
    readInputFile(args.inputFileName);

    START = times(&tmsstart);

    printf("Mutexes Initializing...\n");
    mutex_init(&threadMutex);
    mutex_init(&resourceMapMutex);
    mutex_init(&monitorMutex);

    printf("Creating monitor thread...\n");
    createMonitorThread(args.monitorTime);
    printf("Creating task threads...\n");
    createTaskThreads();
    delay(400); // delay long enough for threads array to be initialized

    printf("Waiting for tasks to finish...\n");
    waitForTaskTermination();

    printf("Tasks Finished...\n");
    printTerminationInfo();
    return EXIT_SUCCESS;
    }