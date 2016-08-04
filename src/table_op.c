#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <table_types.h>
#include <table_kernel.h>
#include <table_high.h>
#include <myregex.h>
#include <globals.h>

#include <assert.h> //For debugging

//Converts 'string' from string to its proper type.
void *type_data_from_string(char *string, TABLE_FIELD *field){
	void *aux = malloc(field->dataSize);
	switch(field->fieldType){
		case INT:
			*(int *) aux = atoi(string);
			return aux;
		case FLOAT:
			*(float *) aux = (float) atof(string);
			return aux;
		case DOUBLE:
			*(double *) aux = atof(string);
			return aux;
		case CHAR:
			*(char *) aux = *string;
			return aux;
		case STRING:
			strncpy((char *) aux, string, field->dataSize);
			return aux;
	}
	free(aux);
	return NULL;
}

//Given a string with multiple substrings separated by commas, return an array of these substrings.
//For our convenience:
//	- trailing and leading 'white' characters are ignored.
//	- 'white' characters in the middle of the substring are not allowed,
//		unless it's surrounded by single quotes.
//	- if surrounded by single quotes, everything surrounded will be considered as the substring to be returned.
char **split_commas(char *s, int *count){
	int slen, mlen, index = 0;
	char **m, **r = NULL;

	slen = strlen(s);
	*count = 0;

	do{
		m = match(s+index, "^\\s*'", 1);
		if(!m){
			m = match(s+index, "^\\s*(\\w+\\.*\\w*)\\s*[,]{0,1}", 2);
		} else {
			matrix_free((void **) m, 1);
			m = match(s+index, "^\\s*'([^']*)'[,]{0,1}", 2);
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

//Converts each 'value_strings[i]', which is related to the field 'fields[i]', to its proper type.
//Returns as a different 
void **strings_to_values(char *tablename, char **fields, char **value_strings, int nvalues){
	int i;
	TABLE_FIELD *curr_field;
	void **result = malloc(sizeof(void *) * nvalues);
	for(i = 0; i < nvalues; i++){
		curr_field = field_from_file(tablename, fields[i]);
		result[i] = type_data_from_string(value_strings[i], curr_field);
		free(curr_field);
	}
	return result;
}

//Creates a table with name 'tablename' and fields specified by the half-parsed string 'params'.
//'params' would be a list of comma-separated of strings that follow the format "{fieldname} {fieldtype} {[datasize]?}"
//e.g: "code int, name char[80], age double, height float"
void shell_table_create(char *tablename, char *params){
	char **m, **fieldNames = NULL;
	FIELD_TYPE type, *fieldTypes = NULL;
	int index = 0, nfields = 0, *dataSizes = NULL, size = strlen(params);

	do{
		m = match(params+index, "^\\s*(\\w+)\\s+(\\w+)\\s*\\[{0,1}\\s*([[:digit:]]*)\\s*\\]{0,1}\\s*[,]{0,1}", 4);
		if(!m) break;
		
		nfields++;
		fieldNames = (char **) realloc(fieldNames, sizeof(char *) * nfields);
		fieldTypes = (FIELD_TYPE *) realloc(fieldTypes, sizeof(FIELD_TYPE) * nfields);
		dataSizes = (int *) realloc(dataSizes, sizeof(int) * nfields);
		
		type = 	strcmp(m[2], "int") == 0 ? INT :
			strcmp(m[2], "float") == 0 ? FLOAT :
			strcmp(m[2], "double") == 0 ? DOUBLE :
			strcmp(m[2], "char") == 0 ? strlen(m[3]) == 0 ? CHAR : STRING :
			-1;
		fieldTypes[nfields-1] = type;
		dataSizes[nfields-1] = 	type == STRING ? atoi(m[3])+1 :
			   		type == CHAR ? sizeof(char) :
			   		type == INT ? sizeof(int) :
			   		type == FLOAT ? sizeof(float) :
					type == DOUBLE ? sizeof(double) : -1;
		fieldNames[nfields-1] = strdup(m[1]);
		
		index += strlen(m[0]);
		matrix_free((void **) m, 4);
	} while(index < size);

	table_create(tablename, nfields, fieldNames, fieldTypes, dataSizes);

	matrix_free((void **) fieldNames, nfields);
	free(fieldTypes);
	free(dataSizes);
}

//Inserts value into table of name "tablename" based on the
//	half-parsed strings 'fields_string' and 'values_string'
//"fields_string" example: "code, name, height, age"
//"values_string" example: "1, 'john smith', 1.57, 19.7"
void shell_table_insert(char *tablename, char *fields_string, char *values_string){
	int countf, countv;
	char **fields, **values_sp;
	void **values;
	
	fields = split_commas(fields_string, &countf);
	values_sp = split_commas(values_string, &countv);

	if(countv == countf){
		values = strings_to_values(tablename, fields, values_sp, countv);
		table_insert(tablename, fields, values);
	} else {
		printf("There was a problem parsing the given command. Try again.\n");
	}

	matrix_free((void **) fields, countv);
	matrix_free((void **) values_sp, countv);
	matrix_free((void **) values, countv);
}

//Converts 'value' to the correct type, based on its field of name 'fieldname'
//then, with said value as key, executes a select procedure upon table of name 'tablename'.
void shell_table_select(char *tablename, char *fieldname, char *value){
	TABLE_FIELD *field = field_from_file(tablename, fieldname);
	void *value_bytes = type_data_from_string(value, field);
	select_records(tablename, fieldname, value_bytes);
	free(value_bytes);
	free(field);
}
