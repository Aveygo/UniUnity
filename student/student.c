/*******************************************************************************
 * student.c
 * Handles managing student preferences
*******************************************************************************/

/*******************************************************************************
 * Custom header files
*******************************************************************************/

#include "../utils/utils.h"

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "student.h"

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

/**
 * Checks if all the preferences contain student ids that are in the 
 * students array
 * Return: int, 0 for fail, 1 for success
 *
 * Inputs
 *  - students      the array of students
 *  - num_students  the number of students in the list
 * Outputs
 *  - 0/1 -> fail or pass respectively 
 * 
 */
int sanity_check_students(Student* students, int num_students) {
    
    /* Allocate a list of the known student ids */
    int* known_student_ids = (int*)malloc(num_students * sizeof(int));

    /* Fill in the known student ids */
    for (int i = 0; i < num_students; i++) {
        known_student_ids[i] = students[i].student_id;
    }

    if (DEBUG) {
        printf("[DEBUG] Known student ids: ");
        for (int i = 0; i < num_students; i++) {
            printf("%d ", known_student_ids[i]);
        }
        printf("\n");
    }

    /* Sort the students for binary search speed boost ?*/
    sort(known_student_ids, num_students);

    /* Finally start checking for any misconfigured student preferences */
    for (int i = 0; i < num_students; i++) {


        /* See if the student preferences are too large */
        if (students[i].preferences_size > MAX_STUDENT_PREFERENCES) {
            if (DEBUG) {
                printf("[ERROR] Too many preferences!:\n");
                printf("\tStudent index: %d\n", i);
                printf("\tNumber of preferences: %d\n", students[i].preferences_size);
            }

            free(known_student_ids);
            return 0;
        }

        /* Check if all the preferences exists */
        for (int j = 0; j < students[i].preferences_size; j++) {
            
            if (fast_search(students[i].preferences[j], known_student_ids, num_students) == -1) {

                if (DEBUG) {
                    printf("[ERROR] invalid preference:\n");
                    printf("\tStudent index: %d\n", i);
                    printf("\tStudent ID: %d\n", students[i].student_id);
                    printf("\tTried to find preference: %d\n", students[i].preferences[j]);
                    printf("\tStudent preferences:");
                    for (int k=0; k<students[i].preferences_size;k++) {
                        printf("%d ", students[i].preferences[k]);
                    }
                    printf("\n");

                }

                free(known_student_ids);
                return 0;
            }
        }
    }
    free(known_student_ids);
    return 1;
}


/**
 * Creates a random array of student ids & preferences 
 * Return: Student struct array
 *
 * Inputs
 *  - num_students  the number of students in the list
 * Outputs
 *  - An random array of students
 * 
 */
Student* generate_students(int num_students) {

    /* Initialise array */
    Student* students = (Student*)malloc(num_students * sizeof(Student));
    if (students == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    /* Initialise the students with random ids and NULL preferences */
    for (int i = 0; i < num_students; i++) {
        students[i].student_id = rand() % MAX_STUDENT_ID + MAX_STUDENT_ID;
        students[i].preferences_size = 0;

        for (int j = 0; j < MAX_STUDENT_PREFERENCES; j++) {
            students[i].preferences[j] = INT_MAX;
        }

    }

    if (DEBUG) {
        printf("[DEBUG] Allocated and initialised %d students\n", num_students);
    }

    /* Go back into the array and fill in the student preferences with random students */
    for (int i = 0; i < num_students; i++) {

        if (DEBUG) {
            printf("[DEBUG] Gave %d preferences:", students[i].student_id);
        }

        /* Each student can have a random number of preferences [0, MAX_STUDENT_PREFERENCES] */
        int random_num_student_preferences = rand() % (MAX_STUDENT_PREFERENCES + 1);

        /* Compute valid preferences (can't be self or same) */
        while (students[i].preferences_size < random_num_student_preferences) {
            int possible_preference = students[rand() % num_students].student_id;

            /* Check if it's already in the preferences */
            if (simple_search(possible_preference, students[i].preferences, students[i].preferences_size) == -1) {
                
                /* Check if its ourselves */
                if (students[i].student_id != possible_preference) {
                
                    /* Add to our preferences */
                    students[i].preferences[students[i].preferences_size] = possible_preference;
                    students[i].preferences_size++;

                    if (DEBUG) {
                        printf(" %d", possible_preference);
                    } 
                }
            }

        }

        if (DEBUG) {
            printf("\n");
        }

    }

    if (DEBUG) {
        printf("[DEBUG] Successfully generated %d students!\n", num_students);
        if (!sanity_check_students(students, num_students)) { 
            printf("[ERROR] Student array is INVALID! Bad preferences!\n");
        } else {
            printf("[DEBUG] Student array passed the preference test, looks valid!\n");
        }
    }

    return students;
}

/**
 * Mainly for debugging, shows the generated students
 * Return: void
 *
 * Inputs
 * - students       array of student struct
 * - num_students   the number of students to generate      
 * Outputs
 *  - Student ids and preferences to stdout
 * 
 */
void display_students(Student* students, int num_students) {
    printf("Current students:\n");
    for (int i = 0; i < num_students; i++) {
        printf("Student %d, Preferences: ", students[i].student_id);
        for (int k = 0; k < students[i].preferences_size; k++) {
            printf("%d ", students[i].preferences[k]);
        }
        printf("\n");
    }
}

/**
 * Convert csv file to students array
 * Return: int, 0 success, 1 failed
 *
 * Inputs
 * - filename       file to load from
 * - new_students   address to store loaded students
 * - num_students   pointer to number of loaded students
 * Outputs
 *  - Student ids and preferences from file
 * 
 */
int load_students_from_csv(char * filename, Student ** new_students, int * num_students) {

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        
        /* Work with the line read from the file */
        int i = 0;
        char *token = strtok(line, ",");
        int integers[1 + MAX_STUDENT_PREFERENCES] = {INT_MAX};

        while (token != NULL && i < 1 + MAX_STUDENT_PREFERENCES) {
            integers[i++] = atoi(token);
            token = strtok(NULL, ",");
        }

        /* Check if all ints were populated */
        /* Obsolete code? */
        /*
            for (int i=0; i<1+MAX_STUDENT_PREFERENCES; i++) {
                if (integers[i] == 0) {        
                    fclose(file);
                    return 1;
                }
            }
        */

        /* Resize to fit new student */
        Student* new_ptr = (Student*)realloc(*new_students, sizeof(Student) * (*num_students + 1));
        
        if (new_ptr == NULL) {
            fclose(file);
            return 1;
        }

        /* Add the student and their preferences into the array */
        *new_students = new_ptr;
        (*new_students)[*num_students].student_id = integers[0];
        (*new_students)[*num_students].preferences_size = 0;

        for (i = 0; i < MAX_STUDENT_PREFERENCES; i++) {
            (*new_students)[*num_students].preferences[i] = integers[i+1];
            if (integers[i+1] != INT_MAX) {
                (*new_students)[*num_students].preferences_size +=1;
            }
        }

        *num_students += 1;
    }

    /* Close and return success */
    fclose(file);
    return 0;
}