/*******************************************************************************
 * solver.c
 * Converts an array of group struct and moves students around till solved 
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "solver.h"

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

/**
 * Finds how many student preferences (as a percentage) are satisfied
 * Return: float
 *
 * Inputs
 *  - student   The student to find the happiness of
 *  - group     The group that the student is in
 * Outputs
 *  - [0-1] Float of student happiness 
 * 
 */
float student_happiness(Student student, Group group) {
    
    if (student.preferences_size == 0) {
        return 1;
    }

    float num_satisfied = 0;
    
    for (int i=0; i<student.preferences_size; i++) {

        /* Search within group if preference was met */
        for (int j=0; j<group.group_size; j++) {
            if (student.preferences[i] == group.students[j]->student_id) {
        
                /* Increment the number of preferences satisfied */
                num_satisfied++;
                
            }
        }
    }
    
    /* Return average */
    return num_satisfied / (float) (student.preferences_size);
}

/**
 * Finds the average happiness of a group
 * Return: float
 *
 * Inputs
 *  - group     Pointer to target group
 * Outputs
 *  - Returns the newly calculated in group happiness
 * 
 */
float set_group_happiness(Group *group) {
    float sum_of_scores = 0;
    float num_of_scores = 0;

    /* For each student, compute their happiness */
    for (int i=0; i<group->group_size; i++) {
        num_of_scores = num_of_scores + 1;
        sum_of_scores = sum_of_scores + student_happiness(*group->students[i], *group);
    }

    if (num_of_scores == 0) {
        group->happiness = 0;
        return 0;
    }
    group->happiness = sum_of_scores / num_of_scores;
    return group->happiness;
}


/**
 * Finds two students that can be swapped
 * Return: void
 *
 * Inputs
 *   groups             Array of group struct
 *   number_of_groups   the number of elements in groups
 *   g1                 The first group index
 *   g2                 The second group index
 *   s1                 The index of the student in g1
 *   s2                 The index of the student in g2 
 * Outputs
 *  - Finds two students that can be swapped
 * 
 */
void compute_proposal(Group* groups, int number_of_groups, int* g1, int* g2, int* s1, int* s2) {

    int valid_swap = 0;
    while (!valid_swap) {

        /* Get two groups */
        *g1 = (int) (rand() % (number_of_groups));
        *g2 = (int) (rand() % (number_of_groups));

        /* Make sure they're not the same group */
        if (*g1 != *g2) {

            /* Be bias against "solved" groups */
            if ((groups[*g1].happiness < 0.9 && groups[*g2].happiness < 0.9) || 0.8 < (double)rand() / (double)RAND_MAX) {

                /* Get two students */
                *s1 = (int) (rand() % groups[*g1].group_size);
                *s2 = (int) (rand() % groups[*g2].group_size);

                /* no need to check if they are the same as they are in different groups */
                valid_swap = 1;

            }
        }
    }
}

/**
 * Swaps two students
 * Return: void
 *
 * Inputs
 *   groups     Array of group struct
 *   g1         The first group index
 *   g2         The second group index
 *   s1         The index of the student in g1
 *   s2         The index of the student in g2 
 * Outputs
 *  - Swaps two students in the groups array
 * 
 */
void swap_students(Group* groups, int g1, int g2, int s1, int s2) {
    Student* student_1 = groups[g1].students[s1];
    Student* student_2 = groups[g2].students[s2];
    groups[g1].students[s1] = student_2;
    groups[g2].students[s2] = student_1;
}

/**
 * Tries to determine if a student swap is beneficial
 * Return: float
 *
 * Inputs
 *   groups             Array of group struct
 *   number_of_groups   The number of elements in groups
 *   p                  The chance (eg 0.01) to keep bad 
 *                      swaps to escape local minima
 * Outputs
 *  - "improved" group array
 * 
 */
float iter(Group* groups, int number_of_groups, float p) {
    
    /* Find two students to swap */
    int g1; int g2; int s1; int s2;
    compute_proposal(groups, number_of_groups, &g1, &g2, &s1, &s2);
    
    /* Remember the old scores in case we need to undo the swap */
    float old_g1_h = groups[g1].happiness;
    float old_g2_h = groups[g2].happiness;

    /* Swap them */
    swap_students(groups, g1, g2, s1, s2);
    
    /* Update happiness scores*/
    float new_g1_h = set_group_happiness(&groups[g1]);
    float new_g2_h = set_group_happiness(&groups[g2]);

    /* Check if it wasn't beneficial */
    float delta = (new_g1_h - old_g1_h) + (new_g2_h - old_g2_h);
    double random_p = (double)rand() / (double)RAND_MAX;
    if (delta < 0 && p < random_p) {

        /* Swap students back to their original positions */
        swap_students(groups, g1, g2, s1, s2);

        /* Undo happiness scores*/
        groups[g1].happiness = old_g1_h;
        groups[g2].happiness = old_g2_h;

        return 0;
    }

    return delta;

}

/**
 * Finds the worst group and prints them to stdout,
 * mainly for debugging
 * Return: void
 *
 * Inputs
 *   groups             Array of group struct
 *   number_of_groups   The number of elements in groups
 * Outputs
 *  - Worst group output in stdout
 * 
 */
void print_worst_group(Group* groups, int number_of_groups) {
    float worst_score = groups[0].happiness;
    int worst_index = 0;

    for (int i=0; i< number_of_groups; i++) {
        if (groups[i].happiness < worst_score) {
            worst_score = groups[i].happiness;
            worst_index = i;
        }
    }

    printf("[DEBUG] Group %d had the worst score of: %lf\n", worst_index, worst_score);
}

/**
 * Tries to move students around and maximize happiness
 * Return: int, 0 fail, 1 success
 *
 * Inputs
 *   groups             Array of group struct
 *   number_of_groups   The number of elements in groups
 * Outputs
 *  - Updated groups, success state
 * 
 */
int solve(Group* groups, int number_of_groups, int confidence, float p, int group_size) {

    /*
    if (number_of_groups < 2) {
        printf("Group size too small, nothing to do...\n");
        return 0;
    }
    */

    /* Compute the initial group scores */
    float scores_sum = 0;
    for (int i=0; i<number_of_groups; i++) {
        scores_sum += set_group_happiness(&groups[i]);
    }

    if (DEBUG) {
        printf("[DEBUG] Initial average group score: %lf\n", scores_sum / number_of_groups);
    }

    /* 
        Convert confidence to the number of iterations.
        Theres quite a bit of maths that is needed to come up with this,
        so just trust me bro.
    */
    int num_iter = (1.5 + confidence * confidence) * number_of_groups * group_size;

    /* Iterate swapping students */
    for (int i=0; i<num_iter; i++) {
        scores_sum += iter(groups, number_of_groups, p);
    }

    if (DEBUG) {
        printf("[DEBUG] Final score: %lf\n", scores_sum / number_of_groups);
        print_worst_group(groups, number_of_groups);
    }

    return 1;
}