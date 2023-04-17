// This program represents the main entry point for a4w23w23tasks. It receives arguments from the command line
// and processes them accordingly to perform a specific task.

#include "parsers.h"
#include "task_manager.h"

/**
    Validates the input arguments and returns an integer exit status.
    @param argumentCount Number of command-line arguments
    @param argumentValues Array of command-line arguments
    @return {@code int} Exit status
    */
    int main(int argumentCount, char *argumentValues[]) {

    // Validate the input arguments and store the result in status
    int status = args_check(argumentCount, argumentValues);

    // If validation is successful, parse the arguments and execute the task. Otherwise, return the status code.

    if (!status) {
    // Parse the input arguments and execute the task
    return run(parse_arguments(argumentCount, argumentValues));
    }

    // If validation fails, return the status code
    return status;
    }