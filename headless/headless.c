/*******************************************************************************
 * headless.c
 * Simplified version of core actions, made for terminal use 
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "headless.h"

#include "../student/student.h"     /* load_students_from_csv display_students*/
#include "../group/group.h"         /*create_initial_groups csv_groups*/
#include "../solver/solver.h"       /*solve*/

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

int headless_mode(char* arg_input_file, char * arg_output_file, int max_group_size) { 

    /* Load student preferences */ 
    Student* new_students = (Student*)malloc(sizeof(Student) * 0);
    int num_students = 0;
    int load_result = load_students_from_csv(arg_input_file, &new_students, &num_students);

    /* Check if the students were loaded correctly */
    if (load_result == 1) {
        free(new_students);
        printf("Invalid input file data\n");
        return 1;
    }

    /* Make sure that there is enough data to use */
    if (num_students < max_group_size * 2) {
        free(new_students);
        printf("Not enough students\n");
        return 1;
    }

    if (DEBUG) {display_students(new_students, num_students);}

    /* Convert the students into groups */
    int number_of_groups;
    Group* groups = create_initial_groups(new_students, num_students, &number_of_groups, max_group_size);

    if (number_of_groups < 2) {
        printf("Group size too small, nothing to do...\n");
        return 1;
    }

    if (number_of_groups < 2) {
        printf("Group size too small, nothing to do...\n");
        return 1;
    }

    /* Make sure that groups was allocated correctly */
    if (groups == NULL) {
        free(new_students);
        printf("Could not create groups\n");
        return 1;
    }

    /* Solve the groups */
    int solved = solve(groups, number_of_groups, 3, 0.00005, max_group_size);

    /* Check if the solver worked */
    if (solved != 1){
        free(new_students);
        for (int current_group_idx=0; current_group_idx<number_of_groups; current_group_idx++) {
            free(groups[current_group_idx].students);
        }
        free(groups);
        
        printf("Could not solve\n");
        return 1;
    }

    /* Save the results */
    int success = csv_groups(groups, number_of_groups, arg_output_file, max_group_size);

    /* Check if we could save */
    if (success == 0) {
        free(new_students);
        for (int current_group_idx=0; current_group_idx<number_of_groups; current_group_idx++) {
            free(groups[current_group_idx].students);
        }
        free(groups);
        printf("Could not save\n");
        return 1;
    }
    
    /* Free and exit */
    free(new_students);
    for (int current_group_idx=0; current_group_idx<number_of_groups; current_group_idx++) {
        free(groups[current_group_idx].students);
    }
    free(groups);
    return 0;
}