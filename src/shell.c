#include <stdlib.h>
#include <stdio.h>
#include <myregex.h>
#include <utils.h>
#include <table_op.h>
#include <stats.h>

//Comment below to ignore case on names of tables or table_fields.
//#define ICASE

#ifdef ICASE
#define CHK_CASE(X) to_lowercase(X)
#else
#define CHK_CASE(X)
#endif

#include <errno.h>	//debugging only

//Comment line below to prevent debug prints.
//#define NDEBUG

#ifdef NDEBUG
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

typedef enum {
	CREATE_TABLE,
	INSERT_TABLE,
	INDEX_TABLE,
	SORT_TABLE,
	SELECT_TABLE,
	PRINT_TABLE,
	PRINT_ALL_TABLES,
	PRINT_ALL_INDEXES,
	STATISTICS,
	QUIT,
	FAILURE
} TABLE_OP;

TABLE_OP parse(char *cmd, char ***s){
	//Extra parentheses to comply with gcc -Wall flag.
	if((*s = reg_parse(cmd, "\\s*create\\s+table\\s+(\\w+)\\s*\\((.+)\\)\\s*;\\s*$", 3)))
		return CREATE_TABLE;
	if((*s = reg_parse(cmd, "\\s*insert\\s+into\\s+(\\w+)\\s*\\((.+)\\)\\s*values\\s*\\((.+)\\)\\s*;\\s*$", 4)))
		return INSERT_TABLE;
	if((*s = reg_parse(cmd, "\\s*create\\s+index\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;\\s*$", 3)))
		return INDEX_TABLE;
	if((*s = reg_parse(cmd, "\\s*sort\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;\\s*$", 3)))
		return SORT_TABLE;
	if((*s = reg_parse(cmd, "\\s*select\\s+(\\w+)\\s+(\\w+)\\s+'\\s*(.*)\\s*'\\s*;\\s*$", 4)))
		return SELECT_TABLE;
	if((*s = reg_parse(cmd, "\\s*print\\s+table\\s+(\\w+)\\s*;\\s*$", 2)))
		return PRINT_TABLE;
	if((*s = reg_parse(cmd, "\\s*showalltable(s){0,1}\\s*(;){0,1}\\s*$", 1)))
		return PRINT_ALL_TABLES;
	if((*s = reg_parse(cmd, "\\s*showallindex(es){0,1}\\s*(;){0,1}\\s*$", 1)))
		return PRINT_ALL_INDEXES;
	if((*s = reg_parse(cmd, "\\s*statistic(s){0,1}\\s*(;){0,1}\\s*$", 1)))
		return STATISTICS;
	if((*s = reg_parse(cmd, "^\\s*quit\\s*;\\s*$", 1)))
		return QUIT;
	return FAILURE;
}

void shell(FILE *stream){
	char *cmd, **s;
	int size;

	//EOF detection for stdin will not work when the user is typing commands.
	//Therefore, this shell() only works with stdin if an input is given through '<' command (linux).
	for(; !file_EOF(stream) ;){
		cmd = read_line(stream);
		switch(parse(cmd, &s)){
			case CREATE_TABLE:
				DEBUG("CREATE_TABLE\n");
				CHK_CASE(s[1]);
				CHK_CASE(s[2]);
				shell_table_create(s[1], s[2]);
				alltables_add(s[1]);
				stats.nTables++;
				size = 3;
				break;
			
			case INSERT_TABLE:
				DEBUG("INSERT_TABLE\n");
				CHK_CASE(s[1]);
				CHK_CASE(s[2]);
				shell_table_insert(s[1], s[2], s[3]);
				size = 4;
				break;

			case INDEX_TABLE:
				DEBUG("INDEX_TABLE\n");
				CHK_CASE(s[1]);
				CHK_CASE(s[2]);
				stats.nIndexes++;
				table_index(s[1], s[2]);
				table_index_sort(s[1], s[2]);
				size = 3;
				break;

			case SELECT_TABLE:
				DEBUG("SELECT_TABLE {%s} {%s}\n", s[2], s[3]);
				CHK_CASE(s[1]);
				CHK_CASE(s[2]);
				shell_table_select(s[1], s[2], s[3]);
				size = 4;
				break;

			case SORT_TABLE:
				DEBUG("SORT_TABLE\n");
				CHK_CASE(s[1]);
				CHK_CASE(s[2]);
				stats.nSorts++;
				table_index(s[1], s[2]);
				table_index_sort(s[1], s[2]);
				size = 3;
				break;
				
			case PRINT_TABLE:
				DEBUG("PRINT_TABLE\n");
				CHK_CASE(s[1]);
				table_print(s[1]);
				size = 2;
				break;

			case PRINT_ALL_TABLES:
				DEBUG("PRINT_ALL_TABLES\n");
				alltables_print();
				size = 1;
				break;
			
			case PRINT_ALL_INDEXES:
				DEBUG("PRINT_ALL_INDEXES\n");
				allindexes_print();
				size = 1;
				break;

			case STATISTICS:
				DEBUG("STATISTICS\n");
				stats_print();
				size = 1;
				break;

			case QUIT:
				DEBUG("QUIT\n");
				free(*s);
				free(s);
				free(cmd);
				return;

			case FAILURE:
				DEBUG("FAILURE {%s}\n", cmd);
				free(cmd);
				continue;
		}

		free(cmd);
		matrix_free((void **) s, size);
	}
}

int main(int argc, char *argv[]){
	FILE *fp;
	char filename[51];
	stats_set();
	scanf("%s", filename);
	fp = fopen(filename, "r");
	if(!fp){
		fprintf(stderr, "Unable to open file: {%s}\n", filename);
		perror("ERROR:");
		exit(0);
	}
	shell(fp);
	fclose(fp);
	return 0;
}
