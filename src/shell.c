/* NOTES:
 * 	This "table handler" works in a very simple way
 *	The user creates a table, then adds data,\
 *	while they are adding data, we put all that data in a .tmp file\
 *	once the user is done adding data, they can ask for an 'index' service\
 *	then we add the data from the .tmp file to the .dat file, and then index all the data
 *
 *	Why don't we directly add data into the .dat file?
 *	Because if the user is adding data into the .tmp file,\
 *	and they have already some indexed data into the .dat file,
 *	even if they don't ask for an indexing service, they can ask for a 'select' service\
 *	having .tmp and .dat files allows us to use binary search into the .dat file,\
 *	and then use sequential search in the .tmp file.
 *	If we put data directly into the .dat file, then we wouldn't know if the data were\
 *	ordered or not, forcing us to do a sequential search throughout the whole data.
 */

/* SCHEDULE:
 * OPTIMIZATIONS:
 * 	Many functions take the name of the table as argument, and then open the table from the .dat file.
 * 	Consider making them accept a TABLE * argument instead. See if that'd be better.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <myregex.h>
#include <utils.h>
#include <table_op.h>
#include <globals.h>

#include <assert.h>	//For debugging only.

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

void create_table(char **s){
	int index = 0, dataSize, size = strlen(s[2]);;
	char **m;
	TABLE *table = table_create(s[1]);
	FIELD_TYPE type;

	do{
		m = match(s[2]+index, "^\\s*(\\w+)\\s+(\\w+)\\s*\\[{0,1}\\s*([[:digit:]]*)\\s*\\]{0,1}\\s*[,]{0,1}", 4);
		if(!m) break;
		
		type = strcmp(m[2], "int") == 0 ? INT :
			strcmp(m[2], "float") == 0 ? FLOAT :
			strcmp(m[2], "double") == 0 ? DOUBLE :
			strcmp(m[2], "char") == 0 ? strlen(m[3]) == 0 ? CHAR : STRING :
			-1;

		dataSize = type == STRING ? atoi(m[3])+1 :
			   type == CHAR ? sizeof(char) :
			   type == INT ? sizeof(int) :
			   type == FLOAT ? sizeof(float) :
			   type == DOUBLE ? sizeof(double) : -1;

		table_add_field(table, m[1], type, dataSize); 
		
		index += strlen(m[0]);
		matrix_free((void **) m, 4);
	} while(index < size);

	table_to_file(table);
	table_destroy(&table);
}

//Returns a string where:
//	result[0] is the full match from s[0] through the next comma or the end of the string.
//	result[1] is everything between the single quotes.
char **split_commas_op(char **s){
	int size0 = 0, size1 = 0;
	char **res = (char **) calloc(sizeof(char *), 2);
	for(; **s != '\''; (*s)++){	//finds first quote.
		res[0] = (char *) realloc(res[0], sizeof(char) * (size0+1));
		res[0][size0++] = **s;
	}
	res[0] = (char *) realloc(res[0], sizeof(char) * (size0+1));
	res[0][size0++] = **s;
	for((*s)++; **s != '\''; (*s)++){	//finds last quote
		res[0] = (char *) realloc(res[0], sizeof(char) * (size0+1));
		res[1] = (char *) realloc(res[1], sizeof(char) * (size1+1));
		res[0][size0++] = **s;
		res[1][size1++] = **s;
	}
	res[1][size1] = '\0';
	for(; **s != ',' && **s != '\0'; (*s)++){	//finds comma or \0
		res[0] = (char *) realloc(res[0], sizeof(char) * (size0+1));
		res[0][size0++] = **s;
	}
	res[0] = (char *) realloc(res[0], sizeof(char) * (size0+2));
	res[0][size0++] = **s;
	res[0][size0] = '\0';
	return res;
}

char **split_commas(char *s, int *count){
	int slen, mlen, index = 0;
	char **m, **r = NULL;

	slen = strlen(s);
	*count = 0;

	do{
		m = match(s+index, "^\\s*'", 1);
		if(!m){
			m = match(s+index, "^\\s*(\\w+\\.*\\w*)\\s*[,]{0,1}", 2);
			//printf("Matched: {%s}\n", m[1]);
		} else {
			matrix_free((void **) m, 1);
			m = s+index;
			m = split_commas_op((char **) &m);
			//printf("Matched: {%s}\n", m[1]);
		}
		if(!m) break;

		mlen = strlen(m[0]);
		index += mlen;
		
		r = (char **) realloc(r, sizeof(char *) * (*count+1));
		r[*count] = (char *) malloc(sizeof(char) * mlen);
		memcpy(r[*count], m[1], mlen);
		(*count)++;

		matrix_free((void **) m, 2);
	} while(index < slen);

	return r;
}

void insert_table(char **s){
	int countf, countv;
	char **fields, **values;
	
	fields = split_commas(s[2], &countf);
	values = split_commas(s[3], &countv);

	if(countv == countf){
		table_insert(s[1], fields, values);
	} else {
		printf("There was a problem parsing the given command. Try again.\n");
	}

	matrix_free((void **) fields, countv);
	matrix_free((void **) values, countv);
}

TABLE_OP parse(char *cmd, char ***s){
	*s = match(cmd, "\\s*create\\s+table\\s+(\\w+)\\s*\\((.+)\\)\\s*;$", 3);
	if(*s) return CREATE_TABLE;

	*s = match(cmd, "\\s*insert\\s+into\\s+(\\w+)\\s*\\((.+)\\)\\s*values\\s*\\((.+)\\)\\s*;$", 4);
	if(*s) return INSERT_TABLE;

	*s = match(cmd, "\\s*create\\s+index\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;$", 3);
	if(*s) return INDEX_TABLE;

	*s = match(cmd, "\\s*select\\s+(\\w+)\\s+(\\w+)\\s+(\\w+\\.*\\w*)\\s*;$", 4);
	if(*s) return SELECT_TABLE;

	*s = match(cmd, "\\s*sort\\s+(\\w+)\\s*\\((\\w+)\\)\\s*;$", 3);
	if(*s) return SORT_TABLE;

	*s = match(cmd, "\\s*print\\s+table\\s+(\\w+)\\s*;$", 2);
	if(*s) return PRINT_TABLE;

	*s = match(cmd, "\\s*showalltable(s){0,1}\\s*;$", 1);
	if(*s) return PRINT_ALL_TABLES;

	*s = match(cmd, "\\s*showallindex(es){0,1}\\s*;$", 1);
	if(*s) return PRINT_ALL_INDEXES;
	
	*s = match(cmd, "\\s*statistic(s){0,1}\\s*;$", 1);
	if(*s) return STATISTICS;

	*s = match(cmd, "^\\s*quit\\s*;$", 1);
	if(*s) return QUIT;

	return FAILURE;
}

void shell(FILE *stream) {
	char *cmd, **s;
	int size;
	
	printf(">> Type your command\n");
	for(;;){
		printf(">> ");
		cmd = read_line(stream);

		switch(parse(cmd, &s)){
			case CREATE_TABLE:
				printf("CREATE_TABLE\n");
				create_table(s);
				size = 3;
				break;
			
			case INSERT_TABLE:
				printf("INSERT_TABLE\n");
				insert_table(s);
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
				printf("\n");
				select_records(s[1], s[2], s[3]);
				printf("\n");
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
				printf("\n");
				alltables_print();
				printf("\n");
				size = 1;
				break;
			
			case PRINT_ALL_INDEXES:
				printf("PRINT_ALL_INDEXES\n");
				printf("\n");
				allindexes_print();
				printf("\n");
				size = 1;
				break;

			case STATISTICS:
				printf("STATISTICS\n");
				printf("\n");
				stats_print();
				printf("\n");
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

int main(int argc, char *argv[]) {
	stats_set();	
	shell(stdin);
	
	return 0;
}
