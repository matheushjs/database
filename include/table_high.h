#ifndef _TABLE_HIGH_H
#define _TABLE_HIGH_H

void alltables_add(char *);
void allindexes_add(char *, char *);
void table_insert(char *, char **, void **);
void table_index(char *, char *);
void index_sort(char *, char *);
int select_records(char *, char *, char *);
void table_print(char *);
void alltables_print();
void allindexes_print();

#endif
