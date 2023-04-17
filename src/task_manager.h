// The following defines a header file for a Task Manager module in a multithreaded system simulator.

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

// Include necessary header files.
#include "parsers.h"
#include "task.h"
#include <map>
#include <string>

// Declare global variables.
extern std::map<std::string, int> resourceMap; // A map of resources and their availability.
extern std::vector <TASK> taskList; // A vector of tasks.
extern pthread_t threads[NTASKS]; // An array of threads used for executing tasks.

// Declare function for running the system simulation.
int run(CommandLineArguments args);

#endif //TASKMANAGER_H