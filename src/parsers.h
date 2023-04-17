// The following defines macros, types, and function declarations related to parsing and input reading.

#ifndef PARSER_H
#define PARSER_H

// Include necessary header files.
#include <chrono>
#include <string>
#include <tuple>
#include <vector>
#include <errno.h>

// Define macros for various flags.
#define RESOURCE_FLAG "resources"
#define TASK_FLAG "task"
#define COMMENT_FLAG "#"

#define RUN_FLAG "RUN"
#define IDLE_FLAG "IDLE"
#define WAIT_FLAG "WAIT"

// Define a constant for the maximum length of a resource string.
#define MAX_RESOURCE_LENGTH 64

// Use the following namespaces.
using std::string;
using std::vector;

// Define an enum for different line types that can be parsed.
typedef enum 
	{
		INVALID, COMMENT, RESOURCE, TASK_
	} LINE_TYPES;

// Define a struct for holding command line arguments.
typedef struct 
	{
		string inputFileName; // The name of the input file.
		long monitorTime; // The time interval between system monitoring.
		uint iterations; // The number of iterations for which the system will be monitored.
} CommandLineArguments;

// Declare functions that will be defined later.
string getFormattedResourceInfo();
string getFormattedTaskInfo();
int args_check(int argumentCount, char *argumentValues[]);
CommandLineArguments parse_arguments(int argumentCount, char *argumentValues[]);
void readInputFile(const string &inputFileName);

#endif