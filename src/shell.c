#include <stdlib.h>
#include <stdio.h>
#include <myregex.h>
#include <utils.h>
#include <table_op.h>
#include <stats.h>

//Comment below to ignore case on <names of tables> and <names of fields>.
#define ICASE

#ifdef ICASE
#define CHK_CASE2(X, Y) to_lowercase(X), to_lowercase(Y)
#define CHK_CASE(X) to_lowercase(X)
#else
#define CHK_CASE2(X, Y)
#define CHK_CASE(X)
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

//Parses command 'cmd', given by the user, conveniently.
//Stores in 's' all useful tokens taken from command.
//Stores in 'size' the amount of such tokens.
//Returns the operation TABLE_OP identified from the given command.
TABLE_OP parse(char *cmd, char ***s, int *size){
	//Extra parentheses to comply with gcc -Wall flag.
	*size = 3;
	if(( *s = reg_parse(cmd, "\\s*create\\s+table\\s+(\\w+)\\s*\\((.+)\\)\\s*;\\s*$", 3) )){
		CHK_CASE2((*s)[1], (*s)[2]);
		return CREATE_TABLE;
	}
	if(( *s = reg_parse(cmd, "\\s*create\\s+index\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;\\s*$", 3) )){
		CHK_CASE2((*s)[1], (*s)[2]);
		return INDEX_TABLE;
	}
	if(( *s = reg_parse(cmd, "\\s*sort\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;\\s*$", 3) )){
		CHK_CASE2((*s)[1], (*s)[2]);
		return SORT_TABLE;
	}
	
	*size = 4;
	if(( *s = reg_parse(cmd, "\\s*insert\\s+into\\s+(\\w+)\\s*\\((.+)\\)\\s*values\\s*\\((.+)\\)\\s*;\\s*$", 4) )){
		CHK_CASE2((*s)[1], (*s)[2]);
		return INSERT_TABLE;
	}
	if(( *s = reg_parse(cmd, "\\s*select\\s+(\\w+)\\s+(\\w+)\\s+'\\s*(.*)\\s*'\\s*;\\s*$", 4) )){
		CHK_CASE2((*s)[1], (*s)[2]);
		return SELECT_TABLE;
	}
	
	*size = 2;
	if(( *s = reg_parse(cmd, "\\s*print\\s+table\\s+(\\w+)\\s*;\\s*$", 2) )){
		CHK_CASE((*s)[1]);
		return PRINT_TABLE;
	}
	
	*size = 0;
	if(reg_match(cmd, "\\s*showalltable(s){0,1}\\s*(;){0,1}\\s*$"))
		return PRINT_ALL_TABLES;
	if(reg_match(cmd, "\\s*showallindex(es){0,1}\\s*(;){0,1}\\s*$"))
		return PRINT_ALL_INDEXES;
	if(reg_match(cmd, "\\s*statistic(s){0,1}\\s*(;){0,1}\\s*$"))
		return STATISTICS;
	if(reg_match(cmd, "^\\s*quit\\s*;\\s*$"))
		return QUIT;
	return FAILURE;
}

//Reads and processes user input.
//'stream' shouldn't be stdin, unless providing a file through the command line operation '<' (linux).
void shell(FILE *stream){
	char *cmd, **s;
	int size;

	for(; !file_EOF(stream) ;){
		cmd = read_line(stream);
		switch(parse(cmd, &s, &size)){
			case CREATE_TABLE:
				shell_table_create(s[1], s[2]);
				alltables_add(s[1]);
				stats_inc(STATS_NTABLES);
				break;
			
			case INSERT_TABLE:
				shell_table_insert(s[1], s[2], s[3]);
				break;

			case INDEX_TABLE:
				table_index(s[1], s[2]);
				table_index_sort(s[1], s[2]);
				stats_inc(STATS_NINDEXES);
				break;

			case SELECT_TABLE:
				shell_table_select(s[1], s[2], s[3]);
				break;

			case SORT_TABLE:
				table_index(s[1], s[2]);
				table_index_sort(s[1], s[2]);
				stats_inc(STATS_NSORTS);
				break;
				
			case PRINT_TABLE:
				table_print(s[1]);
				break;

			case PRINT_ALL_TABLES:
				alltables_print();
				break;
			
			case PRINT_ALL_INDEXES:
				allindexes_print();
				break;

			case STATISTICS:
				stats_print();
				break;

			case QUIT:
				free(cmd);
				return;

			case FAILURE:
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
	stats_reset();
	scanf("%s", filename);
	fp = fopen(filename, "r");
	if(!fp) die("Unable to open file.\n");
	shell(fp);
	fclose(fp);
	return 0;
}
