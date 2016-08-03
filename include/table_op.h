#ifndef _TABLE_OP_H
#define _TABLE_OP_H

#include <stdio.h>
#include <globals.h>

TABLE *table_create(char *name);
void table_destroy(TABLE **table);
void table_add_field(TABLE *table, char *fieldname, FIELD_TYPE type, int dataSize);
void table_to_file(TABLE *table);
TABLE *table_from_file(char *tablename);
TABLE_FIELD *field_from_file(char *tablename, char *fieldname);

void table_insert(char *tablename, char **fieldnames, char **values);
void table_index(char *tablename, char *fieldname);
void index_sort(char *tablename, char *fieldname);
void table_print(char *tablename);
void idx_select(TABLE *table, TABLE_FIELD *field, FILE *idx_fp, FILE *dat_fp, void *value);
void file_select(TABLE *table, TABLE_FIELD *field, FILE *fp, int init, void *value_bytes);
void select_records(char *tablename, char *fieldname, char *value);

void alltables_print();
void allindexes_print();

#endif
