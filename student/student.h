#ifndef STUDENT_H
#define STUDENT_H

#include "../global/global.h" /* standard libraries, consts, structs */

Student* generate_students(int num_students);
void display_students(Student* students, int num_students);
int sanity_check_students(Student* students, int num_students);
int load_students_from_csv(char * filename, Student ** new_students, int * num_students);

#endif