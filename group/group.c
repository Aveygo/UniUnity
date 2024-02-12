/*******************************************************************************
 * group.c
 * Handles managing students into groups 
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "group.h"

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

/**
 * Print groups to stdout, mainly for debugging
 * Return: void
 *
 * Inputs
 *  - groups                Array of group struct
 *  - number_of_groups      Size of the groups argument
 * Outputs
 *  - Result csv file OR compressed student preferences
 * 
 */
void stdout_groups(Group* groups, int number_of_groups) {
    for (int i = 0; i < number_of_groups; i++) {
        
        printf("[Group %03d] %d students: ", i, groups[i].group_size);

        for (int j = 0; j < groups[i].group_size; j++) {
            printf("\t%d", groups[i].students[j]->student_id);
        }

        printf("\n");
    }
}

/**
 * Saves groups to a csv file
 * Return: int, 0 for fail, 1 for success
 *
 * Inputs
 *  - groups                Array of group struct
 *  - number_of_groups      Size of the groups argument
 *  - filename              Filename to save to
 * Outputs
 *  - Saved groups in a csv file
 * 
 */
int csv_groups(Group* groups, int number_of_groups, char filename[], int max_group_size) {
    
    /* Open the file */
    FILE * file = fopen(filename, "w");
    if (file == NULL) {
        return 0;
    }

    /* 
        The first line in a csv is the column names.
        Should follow:
            groupid,groupsize,member0,member1,...membern
    */
    fprintf(file, "groupid,groupsize,");
    for (int i=0;i<max_group_size;i++) {
        fprintf(file, "member%d", i);
        if (i < max_group_size-1) {
            fprintf(file, ",");
        }
    }
    fprintf(file, "\n");

    /* Writing rows */
    for (int i = 0; i < number_of_groups; i++) {
        
        /* Write the group number and size*/
        fprintf(file, "%d,%d,", i, groups[i].group_size);

        /* Write the members in the group */
        for (int k=0;k<max_group_size;k++) {
            if (k < groups[i].group_size) {
                fprintf(file, "%d", groups[i].students[k]->student_id);
                if (k < max_group_size-1) {
                    fprintf(file, ",");
                }
            }
        }
        fprintf(file, "\n");
    }

    /* Close and return success */
    fclose(file); 
    return 1;
}

/**
 * Create groups array from students
 * Return: groups array
 *
 * Inputs
 *  - students              Array of student struct
 *  - num_students          Size of the students argument
 *  - number_of_groups      Pointer to an int, places the number of groups into it 
 * Outputs
 *  - Saved groups in a csv file
 * 
 */
Group* create_initial_groups(Student* students, int num_students, int* number_of_groups, int max_group_size) {
        
    /* Allocate memory for the expected number of groups */
    Group* groups = (Group*)malloc(sizeof(Group) * (num_students / max_group_size + max_group_size));

    if (groups == NULL) {
        return NULL;
    }

    groups[0].group_size = 0;
    groups[0].happiness = -1; /* -1 means it hasn't been computed yet! */

    int current_group_idx = 0;

    /* For each student, add them into the latest non-full group */
    for (int i = 0; i < num_students; i++) {

        if (groups[current_group_idx].group_size == 0) {
            groups[current_group_idx].students = (Student**)malloc(sizeof(Student**) * max_group_size);
        }
        
        /* Check if the current group can handle another student */
        if (groups[current_group_idx].group_size < max_group_size ) {



            /* Add the student to the group */
            groups[current_group_idx].students[groups[current_group_idx].group_size] = &students[i];
            groups[current_group_idx].group_size++;
        
        } else {
            
            if (DEBUG) {
                printf("[DEBUG] Allocating more memory for a new group\n");
            }

            /* Allocate more memory for the next group */
            current_group_idx++;

            groups[current_group_idx].students = (Student**)malloc(sizeof(Student**) * max_group_size);

            /* Add the student to the newly created group */
            groups[current_group_idx].students[0] = &students[i];
            groups[current_group_idx].group_size = 1;
            groups[current_group_idx].happiness = -1;

        }

        if (DEBUG) {
            printf("[DEBUG] Added student id: %d to group %d\n", students[i].student_id, current_group_idx);
        }   
    }

    /* Convert index to size and return groups array */
    *number_of_groups = current_group_idx + 1;
    return groups;
}

