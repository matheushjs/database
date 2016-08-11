#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <table_types.h>
#include <table_kernel.h>
#include <table_high.h>
#include <myregex.h>

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
//Returns NULL in case of failure:
//	- string 's' has length 0.
//	- or any substring of 's' has a not parseable format.
char **insert_parse_op(char *s, int *count){
	int i;
	char **res = NULL, **m, **split = split_string(s, ',', count);
	
	if(*count) res = (char **) malloc(sizeof(char *) * *count);
	for(i = 0; i < *count; i++){
		if(!reg_match(split[i], "^.*'.*'.*$"))
			m = reg_parse(split[i], "^\\s*(\\w+\\.?\\w*)\\s*$", 2);
		else
			m = reg_parse(split[i], "^\\s*'(.*)'\\s*$", 2);
			
		if(m){
			res[i] = m[1];
			free(m[0]), free(m);
		} else {
			matrix_free((void **) res, *count);
			res = NULL;
			break;
		}
	}
	
	matrix_free((void **) split, *count);
	return res;
}

//Converts each 'value_strings[i]', which is a value related to the field 'fields[i]', to its proper type.
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
	FIELD_TYPE type, *fieldTypes;
	int i, nfields, *dataSizes;
	char **m, **fieldNames, **split = split_string(params, ',', &nfields);

	fieldNames = (char **) malloc(sizeof(char *) * nfields);
	fieldTypes = (FIELD_TYPE *) malloc(sizeof(FIELD_TYPE) * nfields);
	dataSizes = (int *) malloc(sizeof(int) * nfields);
	for(i = 0; i < nfields; i++){
		m = reg_parse(split[i], "^\\s*(\\w+)\\s+(\\w+)[\\s[]*([0-9]*)\\s*\\]?\\s*$", 4);
		if(!m) break;
	
		type = 	reg_match(m[2], "^int$") ? INT :
			reg_match(m[2], "^float$") ? FLOAT :
			reg_match(m[2], "^double$") ? DOUBLE :
			reg_match(m[2], "^char$") ? strlen(m[3]) == 0 ? CHAR : STRING :
			-1;
		fieldTypes[i] = type;
		dataSizes[i] = 	type == STRING ? atoi(m[3])+1 :
			   	type == CHAR ? sizeof(char) :
			   	type == INT ? sizeof(int) :
			   	type == FLOAT ? sizeof(float) :
				type == DOUBLE ? sizeof(double) : -1;
		fieldNames[i] = strdup(m[1]);
		matrix_free((void **) m, 4);
	}
	table_create(tablename, nfields, fieldNames, fieldTypes, dataSizes);

	matrix_free((void **) split, nfields);
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
	
	fields = insert_parse_op(fields_string, &countf);
	values_sp = insert_parse_op(values_string, &countv);

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
	table_select_records(tablename, fieldname, value_bytes);
	free(value_bytes);
	free(field);
}
