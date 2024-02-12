#ifndef GLOBAL_H
#define GLOBAL_H

/*
    Double inclusion is prevented with inclusion guards.
    This allows us to import standard libraries globally when
    also importing constants.
    It may be a little bit of a hack, but these libraries were used in 
    almost every file, which makes this method far more convenient.
*/
#include <stdlib.h> /*malloc, calloc, realloc, free, RAND_MAX */
#include <stdio.h>  /*printf, stdin, fopen, fclose, FILE, fgets, getchar, scanf*/
#include <string.h> /*strlen, strtok, strcmp */

/*
    Constants
*/
#define MAX_STUDENT_ID              1000000         /* Used when randomly generating students */
#define MAX_STUDENT_PREFERENCES     5               /* To allow for 1d conversion */
#define INT_MAX                     2147483647      /* Indicates empty preference */
#define MAX_LINE_LENGTH             1000            /* Limit when reading csv */
#define MAX_USR_STR_INP_LEN         256             /* Password and filename input max lengths */

/* Struct to represent a student */
typedef struct {
    int student_id;
    int preferences[MAX_STUDENT_PREFERENCES];
    int preferences_size;
} Student;

/* Struct to represent a group of students */
typedef struct {
    int group_size;
    Student** students;
    float happiness; /* A happiness of -1 means it hasn't been computed yet */
} Group;

/* Struct to represent a node within huffman tree */
typedef struct node node;
struct node {
    int frequency;
    int id;
    int is_branch;
    node *left,*right,*parent,*child;
};

#endif