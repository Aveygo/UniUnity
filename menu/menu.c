/*******************************************************************************
 * menu.c
 * Allows user to access all features in a cli-friendly manner 
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "menu.h"
#include "../student/student.h" /* display_students sanity_check_students load_students_from_csv generate_students*/
#include "../writer/writer.h" /*check_for_password load_students_bin save_students_bin*/
#include "../group/group.h" /*create_initial_groups csv_groups stdout_groups*/
#include "../solver/solver.h" /*solve*/

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

Student* students = NULL;
Group* groups = NULL;

int num_students = 0;           /* The number of elements in students */
int solved = 0;                 /* If the groups were worked on */
int unsaved_preferences = 0;    /* If the current student preferences are saved */
int number_of_groups = 0;       /* The number of elements in groups */
int unsaved_changes = 0;        /* If the results were saved to disk or not */
int max_group_size = 5;         /* The maximum number of students in a group */

int confidence = 3;             /* Solver parameter, controls number of swaps */
float p = 0.00005;              /* Solver parameter, probability to accept a bad swap */

/*
    NOTES
    This file is very long and void* is used extensively here, so here
    are the rules:

     - Each possible menu is represented as a function pointer.
     - If you want some variables/data to be shared across menu options, then they must be global.
     - A menu option cannot take any arguments, and must be of type: "option" (as per menu header) 
     - All menu options MUST return a function, this is why they all have a void* return type.

    Also, heres the source of the unicodes border that i used to make things look cool:
    https://en.wikipedia.org/wiki/Box-drawing_character


    Current menu tree, please update whenever possible

    entry
        quit
        students_menu
            main_menu
            disk_students
                save_student_menu
                load_student_menu

            import_preferences
            generate_menu
            manual_students
                main_menu
                view_students
                add_student
                del_student
        
        solver_menu
            main_menu
            edit_parameters
                edit_iteration
                edit_prob
        
            solve_menu
        
        results_menu
            main_menu
            view_summary
            show_groups
            save_results
*/

/**
 * Handles user input to select a choice
 * Return: menu function
 *
 * Inputs
 *  - choices       Array of menu functions
 *  - max_choices   length of choices
 * Outputs
 *  - Next menu to go to
 * 
 */
void* enter_choice(option* choices, int max_choices) {
    int input;    
    
    while (1) {
        
        printf(" └╴Choice: ");
        int scanf_result = scanf("%i", &input);
        while (getchar() != '\n');
        
        
        if (scanf_result == 1 && input <= max_choices-1 && input >= 0) {
            printf("\n");
            return choices[input]; 
        } else {
            printf(" ┌╴[ERR] Invalid choice.\n");
        }
    }

}

/**
 * Handles confirm user input
 * Return: 1 for confirm, 0 otherwise
 *
 * Inputs
 *  - nan
 * 
 * Outputs
 *  - If the user selected y or n
 * 
 */
int get_y_n() {
    
    char user_continue;
    while (1) {
        int scanf_result = scanf("%c", &user_continue);
        while (getchar() != '\n');
        if (scanf_result == 1 && (user_continue == 'y' || user_continue == 'n')) {
            if (user_continue == 'y') {
                return 1;
            }
            return 0;
        } else {
            printf(" ├╴Invalid input, please use a 'y' or a 'n' character >");
        }
        
    }
}

/**
 * Gets a number from the user
 * Return: int
 *
 * Inputs
 *  - minimum     Lowest that the user input can be
 * Outputs
 *  - Integer of the user input
 * 
 */
int get_amount(int minimum) {
    int user_amount;
    while (1) {
        int scanf_result = scanf("%i", &user_amount);
        while (getchar() != '\n');

        if (scanf_result == 1 && user_amount>minimum) {
            return user_amount;
        } else {
            printf(" ├╴Invalid input, please enter an number greator than %d >", minimum);
        }
        
    }
}

/**
 * Gets a filename from the user
 * Return: int, 0 fail, 1 success
 *
 * Inputs
 *  - filename      array of char to place result in
 *  - check_exists  If function should check if the file is readable
 * Outputs
 *  - updated filename with user input
 * 
 */
int get_filename(char filename[], int check_exists) {
    int file_name_read = 0; 
    
    while (!file_name_read) {
        printf(" ├╴Enter filename >");
        fgets(filename, MAX_USR_STR_INP_LEN, stdin);
    
        /* Removing the newline character if present */
        int len = strlen(filename);
        if (filename[len - 1] == '\n') {
            filename[len - 1] = '\0';
        }
        
        if (check_exists) {
            /* Check if file exits */
            FILE * file = fopen(filename, "r+");

            if (file == NULL) {
                printf(" ├╴File is missing or invalid, try again? [y/n] >");
                if (get_y_n() == 0) {
                    return 0;
                }
            } else {
                fclose(file);
                file_name_read = 1;
            }
        } else {
            /* Check if the filename is valid by opening it, then deleting */
            FILE * file = fopen(filename, "wb");
            if (file == NULL) {
                printf(" ├╴Could not open file for writing, try again? [y/n] >");
                if (get_y_n() == 0) {
                    return 0;
                }
            } else {
                fclose(file);
                file_name_read = 1;
            }
        }
    }
    return 1;
}

/**
 * Gets a password from the user
 * Return: void
 *
 * Inputs
 *  - password      array of char to place result in
 * Outputs
 *  - updated password with user input
 * 
 */
void get_password(char password[]) {
    printf(" ├╴Enter password >");
    fgets(password, MAX_USR_STR_INP_LEN, stdin);
}

/* menu item, prints students to stdout */
void* view_students() {
    display_students(students, num_students);
    return manual_students;
}

/* menu item, handles user input to add a student */
void* add_student() {
    printf("Add student\n");
    printf(" ├╴At any time, type 0 to exit\n");
    printf(" ├╴Enter student id >");
    
    int student_id_to_add = get_amount(-1);
    while (student_id_to_add >= MAX_STUDENT_ID) {
        printf(" ├╴[!] Student id too large, try something smaller?\n");
        printf(" ├╴Enter student id >");
        student_id_to_add = get_amount(-1);
    }

    if (student_id_to_add==0) {
        printf(" └╴Returning back to students menu...\n");
        return manual_students;
    }

    /* Set default preferences to empty (INT MAX) */
    int student_preferences[MAX_STUDENT_PREFERENCES];
    for (int i=0; i<MAX_STUDENT_PREFERENCES; i++) {
        student_preferences[i] = INT_MAX;
    }

    /* Request and add student prefernces */
    int i;
    for (i=0; i<MAX_STUDENT_PREFERENCES; i++) {
        printf(" ├╴Add another student preference? [y/n] >");
        if (get_y_n() == 0) {
            break;
        } else {

            printf(" ├╴Enter the id of the student preference >");
            int student_preference = get_amount(-1);
            if (student_preference==0) {
                printf(" └╴Returning back to students menu...\n");
                return manual_students;
            }
            student_preferences[i] = student_preference;
        }
    }

    printf(" ├╴Are you sure you want to add this student? [y/n] >");

    if (get_y_n() == 1) {

        /* Allocate memory for the students array */
        if (students == NULL) {
            students = (Student*) malloc(sizeof(Student));
        } else {
            students = (Student*) realloc(students, sizeof(Student) * (num_students + 1));
        }
        
        /* Add the student */
        students[num_students].student_id = student_id_to_add;
        for (int k=0; k<MAX_STUDENT_PREFERENCES; k++) {
            students[num_students].preferences[k] = student_preferences[k];
        }
        students[num_students].preferences_size = i;
        num_students = num_students+1;
        unsaved_preferences = 1;
        
        printf(" ├╴Added the student!\n");
    } else {
        printf(" ├╴Cancelled\n");
    }

    printf(" └╴Returning back to students menu...\n");
    return manual_students;    
}

/* menu item, handles user input to delete a student */
void* del_student() {
    printf("\nDelete Student\n");

    if (students == NULL) {
        printf(" └╴No students, returning back to students menu...\n");
        return manual_students;
    }

    printf(" ├╴At any time, type 0 to exit\n");

    int finished=0;

    while (!finished) {
        printf(" ├╴Enter student id to query >");
        int query_student_id = get_amount(-1);
        if (query_student_id==0) {
            printf(" └╴Returning back to students menu...\n");
            return manual_students;
        }

        /* Try and find the student in the students array */
        int found_student_idx = -1;
        for (int i=0; i<num_students;i++) {
            if (students[i].student_id == query_student_id) {
                found_student_idx = i;
            }
        }

        if (found_student_idx > -1) {
            printf(" ├╴Found student! Are you sure you want to delete %d? [y/n] >", query_student_id);
            if (get_y_n() == 1) {
                
                /* Manually move into new students list */
                Student * new_students = (Student*) malloc(sizeof(Student) * (num_students - 1) );
                int new_students_size = 0;
                for (int i=0; i<num_students;i++) {

                    /* Do not move student to be deleted */
                    if (i != found_student_idx ) {
                        new_students[new_students_size] = students[i];
                        new_students_size++;
                    }
                }

                /* Change student pointer to newly moved one */
                num_students = new_students_size;
                free(students);
                students = new_students;

            } 

            finished = 1;

        } else {
            printf(" ├╴Could not find student\n");
        }
    }    
    return manual_students;
}

/* menu item, configure students */
void* manual_students() {
    printf("\nConfigure students\n");
    option load_paths[4] = {main_menu, view_students, add_student, del_student};
    printf(" ├╴[0] Back to main menu\n");
    printf(" ├╴[1] View\n");
    printf(" ├╴[2] Add\n");
    printf(" ├╴[3] Delete\n");
    return enter_choice(load_paths, 4);
}

/* menu item, handles user input to load student preferences from disk */
void* load_student_menu() {
    printf("Load students\n");
    
    if (unsaved_preferences) {
        printf(" ├╴[!] %d unsaved students in memory\n", num_students);
    }
    printf(" ├╴Are you sure you want to load students from disk [y/n] >");
    if (get_y_n() == 0) {
        printf(" └╴Returning back to students menu...\n");
        return students_menu;
    }    

    char password[MAX_USR_STR_INP_LEN] = {"a"};
    char filename[MAX_USR_STR_INP_LEN];

    if (get_filename(filename, 1) ==0 ){
        printf(" └╴Returning back to students menu...\n");
        return students_menu;
    }

    /*
        File can be identified as password protected if the 
        default password does not work
    */
    if (check_for_password(password, filename) != 0) {
        printf(" ├╴File is password protected!\n");
        get_password(password);

        while (check_for_password(password, filename) != 0) {
            printf(" ├╴Incorrect password/bad file, try again? [y/n] >");
            if (get_y_n() == 0) {
                printf(" └╴Returning back to students menu...\n");
                return students_menu;
            }
            get_password(password);
        }
    
    }
    
    /* Status code 2 is invalid password */
    int status_code = 2;
    
    Student * tmp_students;
    int tmp_num_student;

    while (status_code == 2) { 

        /* Attempt to load students */
        status_code = load_students_bin(&tmp_students, &tmp_num_student, password, filename);

        /* Check if password was wrong */
        if (status_code == 2) {
            printf(" ├╴Incorrect password, try again? [y/n] >");
            if (get_y_n() == 0) {
                printf(" └╴Returning back to students menu...\n");
                return students_menu;
            } else {
                get_password(password);
            }
        }
    }

    /* Handle other status codes */
    if (status_code == 5) {
        printf(" ├╴[!] The file was successfully decompressed but\n");
        printf(" ├╴    the students preferences seem to be invalid!\n");
    } else if (status_code != 0){
        printf(" ├╴Sorry! There was an error reading the file, nothing to do...\n");
        printf(" └╴Returning back to students menu...\n");
        return students_menu;
    }

    
    printf(" ├╴Found %d students (status:%d), load? [y/n] >", tmp_num_student, status_code);
    if (get_y_n() == 0) {
        printf(" └╴Returning back to students menu...\n");
    } else {

        /* Change the student pointer to the loaded one */

        if (students != NULL) {
            free(students);
        }
        
        students = tmp_students;
        num_students = tmp_num_student;
        unsaved_preferences = 0; /* The students are in a known file, no need to resave*/
        printf(" └╴Loaded!\n");
    }

    return students_menu;
}

/* menu item, handles user input to save student preferences to disk */
void* save_student_menu() {
    printf("Save students\n");

    if (unsaved_preferences == 0) {
        printf(" ├╴Students are already saved, do you still wish to continue? [y/n] >");
        if (get_y_n() == 0) {
            printf(" └╴Returning back to students menu...\n");
            return students_menu;
        }
    }

    if (sanity_check_students(students, num_students) == 0) {
        printf(" ├╴Student preferences are invalid, do you still wish to continue? [y/n] >");
        if (get_y_n() == 0) {
            printf(" └╴Returning back to students menu...\n");
            return students_menu;
        }
    }

    char filename[MAX_USR_STR_INP_LEN];
    if (get_filename(filename, 0) == 0 ){
        printf(" └╴Returning back to students menu...\n");
        return students_menu;
    }

    printf(" ├╴Would you like to set a password? [y/n] >");
    char password[MAX_USR_STR_INP_LEN] = {"a"};
    
    if (get_y_n() == 1) {
        get_password(password);
    }

    printf(" ├╴Saving, please wait... >\n");
    save_students_bin(students, num_students, password, filename);
    unsaved_preferences = 0;
    printf(" ├╴Done!\n");
    printf(" └╴Returning back to main menu...\n");

    return main_menu;
}

/* menu item, saves or loads student preferences to/from disk */
void* disk_students() {
    if (students == NULL){
        return load_student_menu;
    }
    
    printf("\nSave/Load student preferences\n");
    
    if (unsaved_preferences && students) {
        printf(" ├╴[!] %d unsaved students in memory\n", num_students);
    }

    option load_paths[4] = {main_menu, save_student_menu, load_student_menu};
    printf(" ├╴[0] Back to main menu\n");
    printf(" ├╴[1] Save\n");
    printf(" ├╴[2] Load\n");
    return enter_choice(load_paths, 3);
}

/* menu item, allows user to edit solver iterations parameters */
void* edit_iteration() {
    printf("Editing Confidence Interval\n");
    printf(" ├╴Higher numbers allow for better results but takes longer to compute\n");
    printf(" ├╴Enter a new value of 0 to cancel changes\n");
    printf(" ├╴Default: 3\n");
    printf(" ├╴Current: %d\n", confidence);
    printf(" ├╴New value >");

    int new_confidence = get_amount(-1);
    if (new_confidence!=0) {
        confidence = new_confidence;        
    }

    if (new_confidence > 5) {
        printf(" ├╴[!] Warning, high confidence. Solving may take a while...\n");
    }

    printf(" └╴Returning back to students menu...\n");
    return edit_parameters;

}

/* menu item, allows user to edit solver probability parameter */
void* edit_prob() {
    printf("\nEditing Probability\n");
    printf(" ├╴The chance for solver to accept bad changes, quite sensitive imo...\n");
    printf(" ├╴Enter a new value of 0 to cancel changes\n");
    printf(" ├╴Default: 50\n");
    printf(" ├╴Current: %d\n", (int)(p * 1000000));
    printf(" ├╴New value >");

    int new_p = get_amount(-1);
    if (new_p!=0) {
        p = (float)new_p / 1000000;    
    }

    printf(" └╴Returning back to students menu...\n");
    return edit_parameters;
}

/* menu item, allows user to edit group size */
void* edit_group_size() {
    printf("\nEditing Group size\n");
    printf(" ├╴The output size of each group\n");
    printf(" ├╴Enter a new value of 0 to cancel changes\n");
    printf(" ├╴Default: 5\n");
    printf(" ├╴Current: %d\n", max_group_size);
    
    if (unsaved_changes || groups != NULL) {
        printf(" ├╴[!] Changing the group size will invalidate and clear your current results!\n");
        printf(" ├╴Are you sure you continue? [y/n] >");
        
        if (get_y_n() == 0) {
            printf(" └╴Returning back to students menu...\n");
            return edit_parameters;
        }
    }

    printf(" ├╴New value >");

    int group_size = get_amount(-1);

    if (group_size!=0) {
        
        /* Free the previous group if it exists*/
        if (groups != NULL) {
            for (int current_group_idx=0; current_group_idx<number_of_groups; current_group_idx++) {
                free(groups[current_group_idx].students);
            }
            free(groups);
            groups = NULL;
        }

        unsaved_changes = 0;

        max_group_size = group_size;    
    }

    printf(" └╴Returning back to students menu...\n");
    return edit_parameters;
}

/* menu item, allows user to edit solver paramters */
void* edit_parameters() {
    printf("\nEdit parameters\n");
    option load_paths[4] = {main_menu, edit_iteration, edit_prob, edit_group_size};
    printf(" ├╴[0] Back to main menu\n");
    printf(" ├╴[1] Confidence\n");
    printf(" ├╴[2] Probability\n");
    printf(" ├╴[3] Group Size\n");
    return enter_choice(load_paths, 4);
}

/* menu item, allows user to import student preferences from csv */
void* import_preferences() {

    printf("\nImport student preferences\n");

    char filename[MAX_USR_STR_INP_LEN];

    if (get_filename(filename, 1) ==0 ){
        printf(" └╴Returning back to students menu...\n");
        return students_menu;
    }

    Student* new_students = (Student*)malloc(sizeof(Student) * 0);
    int new_students_size = 0;
    int success_load = load_students_from_csv(filename, &new_students, &new_students_size);

    if (success_load == 1) {
        free(new_students);
        printf(" │ Could not import students from csv\n");
        printf(" └╴Returning back to students menu...\n");
    }
    
    if (students != NULL) {
        free(students);
    }

    students = new_students;
    num_students = new_students_size;
    unsaved_preferences = 1;

    printf(" │ Successful import!\n");
    printf(" └╴Returning back to students menu...\n");

    return students_menu;
}

/* menu item, allows user to generate student preferences */
void* generate_menu() {

    printf("Generate students\n");

    if (students != NULL){

        printf(" │ [!] It looks like you already have some data ready to go!\n");
        printf(" ├╴Are you sure you want to clear it and continue? [y/n] >");
        
        if (get_y_n() == 0) {
            printf(" └╴Returning back to students menu...\n");
            return students_menu;
        }

        free(students);
        students = NULL;
    }
    
    printf(" ├╴How many students to generate? >");
    num_students = get_amount(max_group_size * 2 - 1);
    students = generate_students(num_students);
    unsaved_preferences = 1;

    return show_num_students;
}

/* menu item, defined seperately in case user tries to solve without data */
void* show_num_students() {
    printf(" └╴Generated %d students!\n", num_students);
    return students_menu;
}

/* menu item, allows user to use solver */
void* solve_menu() {
    printf("Solve the groups\n");
    
    if (students == NULL) {
        printf(" │ [!] It looks like you forgot to add your students.\n");
        printf(" ├╴Would you like to generate some? [y/n] >");
        
        if (get_y_n() == 1) {
            generate_menu();
            printf("Solve the groups [returned]\n");
        } else {
            printf(" └╴Pushing you back to the main menu...\n");
            return main_menu;
        }
    }

    if (unsaved_changes) {
        printf(" │ [!] You have unsaved changes, continuing will clear them.\n");
    }

    printf(" └╴Are you sure you want to solve for %d students? [y/n] >", num_students);

    if (get_y_n() == 0) {
        printf(" └╴Returning back to solver menu...\n");
        return solver_menu;
    }

    if (groups != NULL) {
        /* Groups contain pointer to students*/
        for (int current_group_idx=0; current_group_idx<number_of_groups; current_group_idx++) {
            free(groups[current_group_idx].students);
        }
        free(groups);
        groups = NULL;
    }

    printf("\nSolving...\n");
    unsaved_changes = 0;
    groups = create_initial_groups(students, num_students, &number_of_groups, max_group_size);

    if (number_of_groups < 2) {
        printf(" ├╴Group size too small, nothing to do...\n");
        printf(" └╴Going to results menu...\n");
        
        groups = NULL;
        return results_menu;
    }

    printf(" ├╴Initialized %d new groups...\n", number_of_groups);
    printf(" ├╴Please wait, solving...\n");
    solved = solve(groups, number_of_groups, confidence, p, max_group_size);

    printf(" └╴Done! Going to results menu...\n");
    
    if (solved == 1){
        unsaved_changes = 1;
    }
    return results_menu;
}

/* menu item, allows user view short summary of solver */
void* view_summary() {
    printf("Summary\n");
    if (!solved || (groups == NULL)) {
        printf(" └╴Nothing to show, going back results menu...\n");
        return results_menu;
    }

    float worst_score = groups[0].happiness;
    int worst_index = 0;
    float sum_happiness = 0;

    /* Find the sum of happiness, worst score, and worse index */
    for (int i=0; i< number_of_groups; i++) {
        sum_happiness += groups[i].happiness; 

        if (groups[i].happiness < worst_score) {
            worst_score = groups[i].happiness;
            worst_index = i;
        }
    }

    float num_preferences = 0;
    for (int i=0; i<num_students; i++ ) {
        num_preferences += students[i].preferences_size;
    }    

    float avg_happiness = sum_happiness / number_of_groups;
    float avg_preferences = num_preferences /  num_students;

    printf("Group %d had the worst score of: %.2f\n", worst_index, worst_score);
    printf("On average, %.2f out of %.2f (%.0f%%) perferences were met\n", avg_happiness*avg_preferences, avg_preferences, avg_happiness * 100);

    return results_menu;

}

/* menu item, allows user view solved groups */
void* show_groups() {
    if (groups == NULL) {
        printf("\nNo groups to show!\n");
        return results_menu;
    }
    printf("Groups:\n");
    stdout_groups(groups, number_of_groups);
    return results_menu;
}

/* menu item, allows user to save solved groups to a csv */
void* save_results() {
    printf("Save Results\n");

    if (!unsaved_changes) {
        printf(" └╴No groups to save!\n");
        return results_menu;
    }

    char filename[MAX_USR_STR_INP_LEN];
    if (get_filename(filename, 0) == 0 ){
        printf(" └╴Returning back to results menu...\n");
        return results_menu;
    }

    int success = csv_groups(groups, number_of_groups, filename, max_group_size);

    if (success == 0) {
        printf(" └╴Save failed? Going back to results menu...\n");
        return results_menu;
    }

    unsaved_changes = 0;
    printf(" └╴Saved! Going back to results menu...\n");
    return results_menu;
    
}

/* menu item, allows user to manage results */
void* results_menu() {
    option load_paths[4] = {main_menu, view_summary, show_groups, save_results};
    
    printf("\nResults Menu");
    if (unsaved_changes) {printf("*\n");} else {printf("\n");}
    printf(" ├╴[0] Back to main menu\n");
    printf(" ├╴[1] View summary\n");
    printf(" ├╴[2] Show groups\n");
    printf(" ├╴[3] Save results\n");
    return enter_choice(load_paths, 4);
}

/* menu item, allows user to manage the solver */
void* solver_menu() {
    option load_paths[3] = {main_menu, edit_parameters, solve_menu};
    printf("\nSolver Menu\n");
    printf(" ├╴[0] Back to main menu\n");
    printf(" ├╴[1] Edit/View parameters\n");
    printf(" ├╴[2] Solve!\n");
    return enter_choice(load_paths, 3);
}
/* menu item, allows user to manage students */
void* students_menu() {
    option load_paths[5] = {main_menu, disk_students, import_preferences, generate_menu, manual_students};
    printf("\nStudents Menu\n");
    printf(" ├╴[0] Back to main menu\n");
    printf(" ├╴[1] Load/Save Students\n");
    printf(" ├╴[2] Import preferences\n");
    printf(" ├╴[3] Randomly generate\n");
    printf(" ├╴[4] Manually manage\n");
    return enter_choice(load_paths, 5);
};

/* menu item, mainly for deallocating memory and printing goodbye */
void* quit() {
    
    if (groups != NULL) {
        /* Groups contain pointer to students*/
        for (int current_group_idx=0; current_group_idx<number_of_groups; current_group_idx++) {
            free(groups[current_group_idx].students);
        }

        free(groups);  
        groups = NULL;
        if (DEBUG) {
            printf("[DEBUG] Freed groups\n");
        }
    }

    if (students != NULL) {
        free(students);
        if (DEBUG) {
            printf("[DEBUG] Freed students\n");
        }
        students = NULL;
    }
    
    printf("┌──────────────────────────────────────┐\n");
    printf("│     Goodbye and have a nice day!     │\n");
    printf("└──────────────────────────────────────┘\n");
    return NULL;
}

/* menu item, main menu */
void* main_menu() {
    option menu_paths[4] = {quit, students_menu, solver_menu, results_menu};
    printf("\nMain Menu\n");
    printf(" ├╴[0] Quit\n");
    printf(" ├╴[1] Students\n");
    printf(" ├╴[2] Solver\n");
    printf(" ├╴[3] Results\n");
    return enter_choice(menu_paths, 4);
}

/* menu item, hello message, debug message */
void* entry() {

    printf("┌──────────────────────────────────────┐\n");
    printf("│   __  __     _ __  __     _ __    ♠A │\n");
    printf("│  / / / /__  (_) / / /__  (_) /___ __ │\n");
    printf("│ / /_/ / _ \\/ / /_/ / _ \\/ / __/ // / │\n");
    printf("│ \\____/_//_/_/\\____/_//_/_/\\__/\\_, /  │\n");
    printf("│                              /___/   │\n");
    printf("│  A Student Group Optimizer           │\n");
    printf("└──────────────────────────────────────┘\n");

    if (DEBUG) { printf("\n[DEBUG] You are in debug mode, additional outputs will be shown.\n"); }
    return main_menu;
}

/**
 * Manages switching between menu items
 * Return: int 0
 *
 * Inputss
 *  - nan
 * Outputs
 *  - Switching between menu items, returns success always as errors are handled internally
 * 
 */
int option_handler(void) {
    option input = entry;
    while (input != NULL) {
        input = input(&students, &groups, solved);
    }

    return 0;
}
