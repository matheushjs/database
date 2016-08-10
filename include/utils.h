#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <boolean.h>

char *append_string(const char *, const char *);
char *read_line(FILE *);
char **split_string(char *, const char, int *);
void matrix_free(void **, int);
void append_to_file(void *, int, FILE *);
void append_files(char *, char *);
char *append_strings(int, ...);
int get_file_size(FILE *);
bool file_exist(char *);
bool file_EOF(FILE *);
void *file_get_record(int, int, int, FILE *);

#endif
