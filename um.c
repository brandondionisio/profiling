/**************************************************************
 *
 *                     um.c
 *
 *     Assignment: HW 6: um
 *        Authors: Dan Glorioso & Brandon Dionisio (dglori02 & bdioni01)
 *           Date: 04/11/24
 *
 *     Summary: Implementation of the functions for argument handling for the
 *              um program. These functions first check that the user provided
 *              a single file with a valid number of bits, and then opens the
 *              file and runs the um program.
 * 
 **************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "read_and_execute.h"

/* Declaration for open_or_die function */
static FILE *open_or_die(char *fname, char *mode);

/****************** main *******************
 * 
 * Handles the argument usage and calls the um_driver function to execute the
 * instructions from the program file passed in as an argument.
 *
 * Parameters:
 *         int argc:   number of arguments passed into the program
 *      char *argv[]:  pointer to a char array that holds the arguments passed
 *                     into the function
 * Returns:
 *      an integer deciding the success of the completion of the program
 * Expects:
 *      None
 * Notes:
 *      If the program is not passed the correct number of arguments, it will
 *      print a usage message and exit with a failure status.
 *      The file is opened but not closed in this function and thus, it is
 *      expected for the file to be closed elsewhere.
 *
 ********************************************/
int main(int argc, char *argv[])
{
        /* Declare struct to hold file stats and size of file */
        struct stat statistics;
        size_t size_in_bytes;

        /* Check for correct argument usage */
        if (argc == 2) {
                /* Populates the stat stuct according to file and returns
                 * 0 if successful */
                if (stat(argv[1], &statistics) == 0) {
                        size_in_bytes = statistics.st_size;

                        /* Check if the file size is a multiple of 4 bytes */
                        if (size_in_bytes % 4 == 0) {
                                /* Calculate the number of instructions */
                                size_t num_inst = size_in_bytes / 4;

                                /* Open the file */
                                FILE *fp = open_or_die(argv[1], "r");

                                /* Read in and execute the instructions */
                                um_driver(fp, num_inst);
                        }
                }
        } else {
                /* Print usage message and exit with failure status */
                fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        return EXIT_SUCCESS;
}

/************** FILE *open_or_die *************
 * 
 * Opens a file or exits with an error message if the file cannot be opened.
 *
 * Parameters:
 *      char *fname: name of the file to open
 *      char *mode:  mode to open the file in
 * Returns:
 *      FILE *: file pointer to the opened file
 * Expects:
 *      Valid file name and mode are passed in. If the file cannot be opened,
 *      the program will exit with an error message and an exit faulure status.
 *
 ********************************************/
static FILE *open_or_die(char *fname, char *mode)
{
        FILE *fp = fopen(fname, mode);
        if (fp == NULL) {
                fprintf(stderr, "Error: Could not open file %s\n", fname);
                exit(EXIT_FAILURE);
        }
        return fp;
}
