#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <myregex.h>
#include <utils.h>
#include <table_op.h>
#include <globals.h>

typedef enum {
	CREATE_TABLE,
	INSERT_TABLE,
	INDEX_TABLE,
	SELECT_TABLE,
	SORT_TABLE,
	PRINT_TABLE,
	PRINT_ALL_TABLES,
	PRINT_ALL_INDEXES,
	STATISTICS,
	QUIT,
	FAILURE
} TABLE_OP;

TABLE_OP parse(char *cmd, char ***s){
	*s = match(cmd, "\\s*create\\s+table\\s+(\\w+)\\s*\\((.+)\\)\\s*;\\s*$", 3);
	if(*s) return CREATE_TABLE;

	*s = match(cmd, "\\s*insert\\s+into\\s+(\\w+)\\s*\\((.+)\\)\\s*values\\s*\\((.+)\\)\\s*;\\s*$", 4);
	if(*s) return INSERT_TABLE;

	*s = match(cmd, "\\s*create\\s+index\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;\\s*$", 3);
	if(*s) return INDEX_TABLE;

	*s = match(cmd, "\\s*select\\s+(\\w+)\\s+(\\w+)\\s+(\\w+\\.*\\w*)\\s*;\\s*$", 4);
	if(*s) return SELECT_TABLE;

	*s = match(cmd, "\\s*sort\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;\\s*$", 3);
	if(*s) return SORT_TABLE;

	*s = match(cmd, "\\s*print\\s+table\\s+(\\w+)\\s*;\\s*$", 2);
	if(*s) return PRINT_TABLE;

	*s = match(cmd, "\\s*showalltable(s){0,1}\\s*(;){0,1}\\s*$", 1);
	if(*s) return PRINT_ALL_TABLES;

	*s = match(cmd, "\\s*showallindex(es){0,1}\\s*(;){0,1}\\s*$", 1);
	if(*s) return PRINT_ALL_INDEXES;
	
	*s = match(cmd, "\\s*statistic(s){0,1}\\s*(;){0,1}\\s*$", 1);
	if(*s) return STATISTICS;

	*s = match(cmd, "^\\s*quit\\s*;\\s*$", 1);
	if(*s) return QUIT;

	return FAILURE;
}

void shell(FILE *stream){
	char *cmd, **s;
	int size;
	
	printf(">> Type your command\n");
	for(;;){
		printf(">> ");
		cmd = read_line(stream);

		switch(parse(cmd, &s)){
			case CREATE_TABLE:
				printf("CREATE_TABLE\n");
				shell_table_create(s[1], s[2]);
				size = 3;
				break;
			
			case INSERT_TABLE:
				printf("INSERT_TABLE\n");
				shell_table_insert(s[1], s[2], s[3]);
				size = 4;
				break;

			case INDEX_TABLE:
				printf("INDEX_TABLE\n");
				stats.nIndexes++;
				table_index(s[1], s[2]);
				index_sort(s[1], s[2]);
				size = 3;
				break;

			case SELECT_TABLE:
				printf("SELECT_TABLE\n");
				shell_table_select(s[1], s[2], s[3]);
				size = 4;
				break;

			case SORT_TABLE:
				printf("SORT_TABLE\n");
				stats.nSorts++;
				table_index(s[1], s[2]);
				index_sort(s[1], s[2]);
				size = 3;
				break;
				
			case PRINT_TABLE:
				printf("PRINT_TABLE\n");
				table_print(s[1]);
				size = 2;
				break;

			case PRINT_ALL_TABLES:
				printf("PRINT_ALL_TABLES\n");
				alltables_print();
				size = 1;
				break;
			
			case PRINT_ALL_INDEXES:
				printf("PRINT_ALL_INDEXES\n");
				allindexes_print();
				size = 1;
				break;

			case STATISTICS:
				printf("STATISTICS\n");
				stats_print();
				size = 1;
				break;

			case QUIT:
				printf("QUIT\n");
				free(*s);
				free(s);
				free(cmd);
				return;

			case FAILURE:
				printf("FAILURE\n");
				free(cmd);
				continue;
		}

		free(cmd);
		matrix_free((void **) s, size);
	}
}

int main(int argc, char *argv[]){
	stats_set();
	shell(stdin);
	return 0;
}
