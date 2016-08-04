#ifndef _TABLE_KERNEL_H
#define _TABLE_KERNEL_H

#include <utils.h>
#include <table_types.h>

int table_root_size(TABLE *);
int field_offset(TABLE *, char *);

void alltables_add(char *);
void allindexes_add(char *, char *);

TABLE *table_alloc(char *);
void table_destroy(TABLE **);
void table_add_field(TABLE *, char *, FIELD_TYPE, int );
void table_to_file(TABLE *table);
void table_create(char *, int, char **, FIELD_TYPE *, int *);
TABLE *table_from_file(char *);
TABLE_FIELD *field_from_file(char *, char *);

void tmp_to_dat(char *);

bool type_higher(void *, void *, TABLE_FIELD *);
bool type_equal(void *, void *, TABLE_FIELD *);
void record_print(void *, TABLE *);

#endif
