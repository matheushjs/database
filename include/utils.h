#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>

typedef enum {
	FALSE,
	TRUE
} bool;

char *append_string(const char *src, const char *append);
char *read_line(FILE *stream);
void matrix_free(void **m, int row);
void append_to_file(void *value, int size, FILE *fp);
void append_files(char *dest, char *src);
char *append_strings(int nstrings, ...);
int get_file_size(FILE *fp);
bool file_exist(char *filename);
bool file_EOF(FILE *fp);
void *file_get_record(int index, int initOffset, int recordSize, FILE *fp);

#endif
