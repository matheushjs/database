#ifndef _TABLE_HIGH_H
#define _TABLE_HIGH_H

#include <boolean.h>

void alltables_add(char *);
void allindexes_add(char *, char *);

void table_insert(char *, char **, void **);
bool table_index(char *, char *);
bool table_index_sort(char *, char *);
int table_select_records(char *, char *, char *);
void table_print(char *);

void alltables_print();
void allindexes_print();

#endif
