#ifndef MENU_H
#define MENU_H

#include "../global/global.h" /* standard libraries, consts, structs */

typedef void* (*option)();
int option_handler(void);

void* main_menu();
void* students_menu();
void* load_students();
void* disk_students();
void* manual_students();
void* solver_menu();
void* results_menu();
void* edit_parameters();
void* solve_menu();
void* show_num_students();
void* add_student();
void* quit();
void* save_student_menu();
void* load_student_menu();
void* import_preferences();
void* generate_menu();
void* edit_iteration();
void* edit_prob();
void* view_summary();
void* show_groups();
void* save_results();

#endif