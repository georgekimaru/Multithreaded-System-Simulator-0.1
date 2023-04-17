// The following defines a struct for representing a task in a system.

#ifndef TASK_H
#define TASK_H

// Include necessary header files.
#include <string>
#include <vector>

// Define constants for the maximum number of tasks and resource types.
#define NTASKS 25
#define NRES_TYPES 10

// Use the following namespace.
using std::string;

// Define an enum for task statuses.
typedef enum {
WAIT, RUN, IDLE
} STATUS;

typedef struct {
char name[100]; // The name of the task.
int busyTime; // The amount of time the task is busy.
int idleTime; // The amount of time the task is idle.
long totalBusyTime; // The total amount of time the task has been busy.
long totalIdleTime; // The total amount of time the task has been idle.
long totalWaitTime; // The total amount of time the task has waited.
vector <string> reqResources; // A vector of strings representing the resources required by the task.
bool assigned; // A flag indicating if the task has been assigned resources.
int timesExecuted; // The number of times the task has been executed.
STATUS status; // The status of the task.
} TASK;

#endif