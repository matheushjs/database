#ifndef _TABLE_OP_H
#define _TABLE_OP_H

#include <stdio.h>
#include <globals.h>

void table_insert(char *tablename, char **fieldnames, void **values);
void table_index(char *tablename, char *fieldname);
void index_sort(char *tablename, char *fieldname);
void table_print(char *tablename);
void idx_select(TABLE *table, TABLE_FIELD *field, FILE *idx_fp, FILE *dat_fp, void *value);
void file_select(TABLE *table, TABLE_FIELD *field, FILE *fp, int init, void *value_bytes);
void select_records(char *tablename, char *fieldname, char *value);

void alltables_print();
void allindexes_print();

char **split_commas(char *s, int *count);
void **strings_to_values(char *tablename, char **fields, char **value_strings, int nvalues);
void shell_table_create(char *tablename, char *params);
void shell_table_insert(char *tablename, char *fields_string, char *values_string);

#endif
