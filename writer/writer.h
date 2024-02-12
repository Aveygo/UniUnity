#ifndef WRITER_H
#define WRITER_H

#include "../global/global.h" /* standard libraries, consts, structs */

int save_students_bin(Student* students, int num_students, char password[], char filename[]);
int load_students_bin(Student ** students, int* result_num_students , char password[], char filename[]);
int check_for_password( char password[], char filename[]);

#endif