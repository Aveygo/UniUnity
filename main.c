/*******************************************************************************
 * Steps to run:
 * make; ./main
 * 
 * Use the --help flag for additional features 
*******************************************************************************/

/*******************************************************************************
 * Standard & Custom header files
*******************************************************************************/

#include "global/global.h"      /* standard libraries, consts, structs */
#include "menu/menu.h"          /* option_handler */
#include "headless/headless.h"  /* headless_mode */

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

int main(int argc, char* argv[]);

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

/**
 * Entry of solution. Either runs in headless, or as menu
 * Return: int, 0 for success, 1 for failed
 *
 * Inputs
 *  - argc     Number of arguments
 *  - argv     Array of arguments that program was run with 
 * Outputs
 *  - Result csv file OR compressed student preferences
 * 
 */
int main(int argc, char* argv[]) {

    /* Arguments definition */
    char arg_debug[8] = "-d";
    char arg_input[8] = "-i";
    char arg_output[8] = "-o";
    char arg_groups[8] = "-g";
    char arg_help[8] = "--help";

    char * arg_input_file;
    char * arg_output_file;
    
    /*
        Headless state, checks if both the input and output file
        was defined
        0 -> no input/output files
        1 -> one out of two defined
        2 -> both input and output
    */
    int headless = 0;
    
    /* size of groups in headless mode */
    int size_of_groups = 5;

    /* Process the provided arguments */
    int i;
    for (i = 1; i < argc; i++) {
        
        /* Help menu */
        if (strcmp(argv[i], arg_help) == 0) {
            printf("usage: main [--help] [-d] ([-i] input_file [-o] output_file ([-g] max_group_size))\n");
            printf("optional arguments:\n");
            printf("--help      show this help message and exit\n");
            printf("-d          debug mode, shows additional data\n");
            printf("-i          input csv of student preferences\n");
            printf("-o          output csv of solved groups\n");
            printf("-g          maximum size of groups in solution\n");
            return 1;
        
        /* Set global debug to true */
        } else if (strcmp(argv[i], arg_debug) == 0) {
            DEBUG = 1;

        /* Input file defined */
        } else if (strcmp(argv[i], arg_input) == 0) {
            headless += 1;
            if (i+1 < argc) {
                arg_input_file = argv[i+1];
            } else {
                printf("No value for input file provided\n");
                return 1;
            }
            i = i+1;
        
        /* Output file defined */
        } else if (strcmp(argv[i], arg_output) == 0) {
            headless += 1;
            if (i+1 < argc) {
                arg_output_file = argv[i+1];
            } else {
                printf("No value for output file provided\n");
                return 1;
            }
            i = i+1;
        }
        
        /* Number of groups declared */
        else if (strcmp(argv[i], arg_groups) == 0) {
            if (i+1 < argc) {
                size_of_groups = atoi(argv[i+1]);
            } else {
                printf("No value for group size provided\n");
                return 1;
            }

            if ( size_of_groups < 1) {
                printf("Invalid group size, must be larger than 1\n");
                return 1;
            } 

            i = i+1;

        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    /* Check if both "headless" values were provided */
    if (headless == 1) {
        printf("Invalid arguments, need both -i and -o to be defined\n");
        return 1;

    } else if (headless != 2) {
        /* Not headless, switch to menu system and exit */
        return option_handler();
    } else {
        return headless_mode(arg_input_file, arg_output_file, size_of_groups);
    }
}