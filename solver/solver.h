#ifndef SOLVER_H
#define SOLVER_H

#include "../global/global.h" /* standard libraries, consts, structs */

int solve(Group* groups, int number_of_groups, int confidence, float p, int group_size);
float student_happiness(Student student, Group group);
float group_happiness(Group group);

#endif