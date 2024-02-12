#ifndef GROUP_H
#define GROUP_H

#include "../global/global.h" /* standard libraries, consts, structs */

Group* create_initial_groups(Student* students, int num_students, int* number_of_groups, int max_group_size);
void stdout_groups(Group* groups, int number_of_groups);
int csv_groups(Group* groups, int number_of_groups, char filename[], int max_group_size);

#endif