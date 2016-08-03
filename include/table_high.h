#ifndef _TABLE_HIGH_H
#define _TABLE_HIGH_H

#include <stdio.h>
#include <globals.h>

void table_insert(char *, char **, void **);
void table_index(char *, char *);
void index_sort(char *, char *);
void table_print(char *);
void idx_select(TABLE *, TABLE_FIELD *, FILE *, FILE *, void *);
void file_select(TABLE *, TABLE_FIELD *, FILE *, int , void *);
void select_records(char *, char *, char *);
void table_print_header(TABLE *);
void table_print_info(TABLE *);
void ftype_table_print(TABLE *, FILE_TYPE);
void table_print(char *);
void alltables_print();
void allindexes_print();

#endif
