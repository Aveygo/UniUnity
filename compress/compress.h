#ifndef COMPRESS_H
#define COMPRESS_H

#include "../global/global.h" /* standard libraries, consts, structs */

int flatten(Student* students, int num_students, int* int_student_arr);
void unflatten(int* students_1d, int num_students, Student * result);
int* find_unique(int* int_student_arr, int int_student_arr_size, int* num_unique);
node* build_initial_frequencies(node* frequencies, int* int_student_arr, int int_student_arr_size, int* known_student_ids, int num_students);
void build_tree(node * top, int num_students, node* frequencies);
void id_to_code(int id, node* root, node * frequencies, int num_students, int* output, int *top);
int codes_to_ids(int* ids, int* codes, int num_codes, node* root);
void serialize_tree(node* root, int* serialized, int *top);
node * deserialize_tree(int* serialized, node* nodes, int *top);

#endif