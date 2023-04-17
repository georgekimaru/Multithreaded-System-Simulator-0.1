#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <map>
#include <stdlib.h>
#include "parsers.h"
#include "task.h"
#include "task_manager.h"


using namespace std;

/**
 * Checks for proper command line arguments and returns 0 on valid
 * args or an error code
 * @param argumentCount
 * @param argumentValues
 * @return status
 */
int args_check(int argumentCount, char *argumentValues[]) {
    if (argumentCount != 4) {
        printf("Invalid number of arguments.\n");
        return EINVAL;
    }

    int monitorTime = atoi(argumentValues[2]);
    int iterations = atoi(argumentValues[3]);

    if (monitorTime < 0) {
        printf("monitorTime invalid\n");
        return EINVAL;
    }

    if (iterations < 0) {
        printf("NITER invalid\n");
        return EINVAL;
    }

    return 0;
}

/**
 * Converts args into their types and returns them in a struct
 * @param argumentCount
 * @param argumentValues
 * @return {@code CommandLineArguments} args
 */
CommandLineArguments parse_arguments(int argumentCount, char *argumentValues[]) {
    CommandLineArguments args;
    args.inputFileName = argumentValues[1];
    args.monitorTime = (atoi(argumentValues[2]));
    args.iterations = (atoi(argumentValues[3]));
    return args;
}

/**
    Generates a formatted string with system resource information for termination output
    @return A string containing the formatted resource information
    */
std::string getFormattedResourceInfo() {
    std::map<string, int>::iterator itr;
    std::string systemResources;

    // Iterates over the resource map and appends each resource's information to the output string
    for (itr = resourceMap.begin(); itr != resourceMap.end(); itr++) {
        char buffer[MAX_RESOURCE_LENGTH];

        // Formats the resource information using the iterator
        int val = sprintf(buffer, "\t%s: (maxAvail=   %i, held=   0) \n", (itr->first).c_str(),
                          resourceMap[itr->first]);

        // Checks if formatting was successful, and exits the program if not
        if (!val) {
            printf("Failed to print term info");
            exit(EXIT_FAILURE);
        }

        // Appends the formatted string to the output string
        systemResources.append(buffer);
    }
    return systemResources;
}

/**
    Converts a STATUS enum value to its corresponding flag string
    @param tStatus The STATUS enum value to convert
    @param status The output buffer for the converted flag string
    */
void convertStatus(STATUS tStatus, char *status) {

    // Sets the flag string based on the given status
    if (tStatus == IDLE) {
        strcpy(status, IDLE_FLAG);
    } else if (tStatus == WAIT) {
        strcpy(status, WAIT_FLAG);
    } else {
        strcpy(status, RUN_FLAG);
    }
}

/**
    Generates a formatted string with information about a system task's resource usage
    @param reqResource The resource string specifying the required resource and quantity
    @param buffer The output buffer for the generated formatted string
    */
void getFormattedSystemTaskResourceInfo(std::string reqResource, char* buffer) {
  char *saveptr;
  char *resourceName;
  int resourcesNeeded;
  char resourceString[MAX_RESOURCE_LENGTH];

  // Converts the input string to a C-style string and extracts the resource name and quantity
  strcpy(resourceString, reqResource.c_str());
  resourceName = strtok_r(resourceString, ":", &saveptr);
  resourcesNeeded = atoi(strtok_r(nullptr, ":", &saveptr));
  
  // Formats the resource information using the extracted values
  sprintf(buffer, "\t %s: (needed=\t%d, held= 0)\n", resourceName, resourcesNeeded);
}

/**
 * Create a string representation of task termination output
 * @return systemTasks - a string of formatted task info
 */
std::string getFormattedTaskInfo() {
    std::string systemTasks;

    // iterate over each task in the task list
    for (unsigned int i = 0; i < taskList.size(); i++) {
        char buffer[1024];
        char status[MAX_RESOURCE_LENGTH];

        // get the task status and convert it to a string
        convertStatus(taskList.at(i).status, status);

        // create a formatted string with task information
        sprintf(buffer, "[%d] %s (%s, runTime= %i msec, idleTime= %i msec):\n", i,
                taskList.at(i).name, status,
                taskList.at(i).busyTime, taskList.at(i).idleTime);

        // append task information to the systemTasks string
        systemTasks.append(buffer);

        // create a formatted string with the task thread ID and append it to the systemTasks string
        sprintf(buffer, "\t (tid= %lu)\n", threads[i]);
        systemTasks.append(buffer);

        // iterate over each required resource for the task
        for (auto &reqResource : taskList.at(i).reqResources) {
            char resBuffer[1024];

            // create a formatted string with the required resource info and append it to the systemTasks string
            getFormattedSystemTaskResourceInfo(reqResource, resBuffer);
            systemTasks.append(resBuffer);
        }

        // create a formatted string with the task execution and wait times and append it to the systemTasks string
        sprintf(buffer, "\t (RUN: %d times, WAIT: %lu msec)\n\n", taskList.at(i).timesExecuted,
                taskList.at(i).totalWaitTime);
        systemTasks.append(buffer);
    }
    return systemTasks;
}

/**
 * Add a resource to the resource map
 * @param arg - a string containing the resource name and value pair
 */
void parseResourceArg(const string &arg) {
    char *saveptr;
    char nameValuePair[MAX_RESOURCE_LENGTH];
    int number;

    // copy the arg string to a cstring
    strcpy(nameValuePair, arg.c_str());

    // separate the name and value pair
    string name(strtok_r(nameValuePair, ":", &saveptr));
    number = atoi(strtok_r(nullptr, ":", &saveptr));

    // add the resource to the resource map
    resourceMap[name] = number;
}

/**
 * Convert a resources line into a map of resources
 * @param line - a string containing the resources to be added to the resource map
 */
void parseResourcesLine(const string &line) {
    char *temp;
    char *saveptr;
    char cline[100];

    // copy the line string to a cstring
    strcpy(cline, line.c_str());
    vector<char *> resourceStrings;

    // go to the first name:value pair
    temp = strtok_r(cline, " ", &saveptr);
    temp = strtok_r(nullptr, " ", &saveptr);

    // iterate through the rest of the string and add each name:value pair to a vector
    while (temp != nullptr) {
        resourceStrings.push_back(temp);
        temp = strtok_r(nullptr, " ", &saveptr);
    }

    // parse each name:value pair in the vector and add it to the resource map
    for (auto &resourceString : resourceStrings) {
        parseResourceArg(resourceString);
    }
}

/**
    Converts a task file line to an item on the task list
    @param line - a string containing the task file line to parse
    */
void parseTaskLine(const string &line) {
    // convert line string to a C-style string
    char cline[100];
    strcpy(cline, line.c_str());

    // use strtok_r to split the string into its components
    char *saveptr;
    char *token;
    token = strtok_r(cline, " ", &saveptr); // flag
    token = strtok_r(nullptr, " ", &saveptr); // id
    TASK newTask;
    newTask.status = IDLE;
    newTask.totalIdleTime = 0;
    newTask.totalBusyTime = 0;
    newTask.totalWaitTime = 0;
    newTask.timesExecuted = 0;
    strcpy(newTask.name, token);
    token = strtok_r(nullptr, " ", &saveptr); // busy
    newTask.busyTime = atoi(token);
    token = strtok_r(nullptr, " ", &saveptr); // idle
    newTask.idleTime = atoi(token);

    // parse resource requirements
    token = strtok_r(nullptr, " ", &saveptr);
    newTask.assigned = false;
    while (token != nullptr) {
    string str(token);
    newTask.reqResources.push_back(str);
    token = strtok_r(nullptr, " ", &saveptr);
    }

    // add new task to the task list
    taskList.push_back(newTask);
    }

/**
    Returns the type of the input file line
    @param line - a string containing the input file line
    @return a LINE_TYPES enum value indicating the type of the line
    */
    LINE_TYPES getInputFileLineType(const string &line) {

    // check for comments or empty lines
    if (!line.length() || line[0] == '#' || line[0] == '\r' || line[0] == '\n') {
    return COMMENT;
    }

    // split the line into components and check the flag to determine the line type
    const char *flag;
    istringstream iss(line);
    vector <string> items((istream_iterator<string>(iss)), istream_iterator<string>());
    flag = items.at(0).c_str();

    if (strcmp(flag, RESOURCE_FLAG) == 0) {
    return RESOURCE;
    }

    if (strcmp(flag, TASK_FLAG) == 0) {
    return TASK_;
    }

    // line is invalid if it doesn't start with a recognized flag
    return INVALID;
    }


/**
    Parses the input file line and performs appropriate actions based on its type.
    @param line The input file line to parse.
    */
void parseInputFileLine(const string& line) {
    switch (getInputFileLineType(line)) {
        case TASK_:
        printf("Parsing task...\n");
        parseTaskLine(line);
    break;
        case RESOURCE:
        printf("Parsing resources...\n");
        parseResourcesLine(line);
    break;
        case COMMENT:
        printf("Ignoring blank/comment line...\n");

    // ignore any comments or white lines
    break;
    default: // INVALID

    printf("ERROR: INVALID LINE: %s\n", line.c_str());
    exit(EINVAL);
    }
    }

/**
    Reads the input file and parses its contents.
    @param inputFileName The name of the input file to read and parse.
    */
    void readInputFile(const string& inputFileName) {
    string line; // line read from file
    ifstream file(inputFileName);

    if (!file.is_open()) { // use is_open() method to check if file opened successfully
    printf("FILE DOES NOT EXIST\n");
    exit(EXIT_FAILURE);
    }

    printf("File opened successfully...\n");

    while (getline(file, line)) {
    parseInputFileLine(line);
    }

    file.close(); // close the file stream after use
    }