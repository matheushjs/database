#ifndef _TABLE_KERNEL_H
#define _TABLE_KERNEL_H

#include <globals.h>

int table_record_size(TABLE *table);
int table_root_size_constant();
int table_root_size(TABLE *table);
int field_offset(TABLE *table, char *fieldname);

void alltables_add(char *tablename);
void allindexes_add(char *tablename, char *fieldname);

TABLE *table_alloc(char *name);
void table_destroy(TABLE **table);
void table_add_field(TABLE *table, char *fieldname, FIELD_TYPE type, int dataSize);
void table_to_file(TABLE *table);
void table_create(char *tablename, int nfields, char **names, FIELD_TYPE *types, int *sizes);
TABLE *table_from_file(char *tablename);
TABLE_FIELD *field_from_file(char *tablename, char *fieldname);

void tmp_to_dat(char *tablename);

bool type_higher(void *value1, void *value2, TABLE_FIELD *field);
bool type_equal(void *value1, void *value2, TABLE_FIELD *field);
void *type_data_from_string(char *string, TABLE_FIELD *field);
void type_append_from_string(char *string, TABLE_FIELD *field, FILE *fp);
void type_value_print(void *value, TABLE_FIELD *field);
char *type_to_string(FIELD_TYPE ftype);

void record_print(void *record, TABLE *table);
void ftype_table_print(TABLE *table, FILE_TYPE ftype);
void table_print_header(TABLE *table);
void table_print_info(TABLE *table);

#endif
