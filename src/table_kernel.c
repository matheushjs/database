#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <globals.h>

//Returns the size of a single record within a file.
int table_record_size(TABLE *table){
	int i, size = 0;
	for(i = 0; i < table->fieldCounter; i++)
		size += table->fields[i]->dataSize;
	return size;
}

//Returns the size of the constant part of the table root.
int table_root_size_constant(){
	TABLE *table;
	return (int) sizeof(table->fieldCounter) +
	       (int) sizeof(table->rootSize) +
	       (int) sizeof(table->recordSize);
}

//The .dat file begins with information about the table, and then the actual data.
//This function returns the 'offset' in which the actual data begins.
int table_root_size(TABLE *table){
	return table_root_size_constant() +
	       table->fieldCounter * (int) sizeof(TABLE_FIELD);
}

//Returns the 'offset' of the data from the field 'fieldname',
//relative to the initial position of the whole record.
int field_offset(TABLE *table, char *fieldname){
	int i = 0, size = 0;
	while(strcmp(fieldname, (char *) table->fields[i]->name) != 0){
		size += table->fields[i++]->dataSize;
		if(i == table->fieldCounter) return -1;
	}
	return size;
}

//Appends data from 'tablename.tmp' to 'tablename.dat'
//then erases all data in the '.tmp' file.
void tmp_to_dat(char *tablename){
	char *dat, *tmp;
	dat = append_string(tablename, ".dat");
	tmp = append_string(tablename, ".tmp");

	append_files(dat, tmp);

	fclose(fopen(tmp, "w"));
	free(dat);
	free(tmp);
}

//Returns TRUE if value1 is higher than value2.
bool type_higher(void *value1, void *value2, TABLE_FIELD *field){
	switch(field->fieldType){
		case INT:
			return *(int *) value1 > *(int *) value2 ? TRUE : FALSE;
		case FLOAT:
			return *(float *) value1 > *(float *) value2 ? TRUE : FALSE;
		case DOUBLE:
			return *(double *) value1 > *(double *) value2 ? TRUE : FALSE;
		case CHAR:
			return *(char *) value1 > *(char *) value2 ? TRUE : FALSE;
		case STRING:
			return strcmp((char *) value1, (char *) value2) > 0 ? TRUE : FALSE;
		default:
			printf("Invalid fieldType in the field passed as argument.\n");
			return FALSE;
	}
}

//Returns TRUE if value1 is equal to value2.
bool type_equal(void *value1, void *value2, TABLE_FIELD *field){
	int len;
	if(field->fieldType == STRING){
		len = strlen(value1);
		return (len == strlen(value2) && !memcmp(value1, value2, len)) ? TRUE : FALSE;
	} 
	return !memcmp(value1, value2, field->dataSize) ? TRUE : FALSE;
}

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

//Converts 'string' to its proper data type.
//Then appends it to the file pointed to by 'fp'.
void type_append_from_string(char *string, TABLE_FIELD *field, FILE *fp){
	void *value = type_data_from_string(string, field);
	append_to_file(value, field->dataSize, fp);
	free(value);
}

//Prints 'value' based on its type.
void type_value_print(void *value, TABLE_FIELD *field){
	switch(field->fieldType){
		case INT:
			printf("%d", *(int *) value);
			break;
		case FLOAT:
			printf("%f", *(float *) value);
			break;
		case DOUBLE:
			printf("%lf", *(double *) value);
			break;
		case CHAR:
			printf("%c", *(char *) value);
			break;
		case STRING:
			printf("%s", (char *) value);
			break;
	}
}

char *type_to_string(FIELD_TYPE ftype){
	char *result = malloc(7);
	strcpy(	result,
		ftype == STRING ? "string" :
		ftype == CHAR ? "char" :
		ftype == INT ? "int" :
		ftype == FLOAT ? "float" :
		ftype == DOUBLE ? "double" : "");
	return result;
}

//Prints a record from the table 'table'.
void record_print(void *record, TABLE *table){
	int i, pos = 0;

	if(table->fieldCounter) printf("|  ");
	for(i = 0; i < table->fieldCounter; i++){
		type_value_print(record+pos, table->fields[i]);
		pos += table->fields[i]->dataSize;
		printf("  |  ");
	}
	printf("\n");
}

//Prints a table from either a .tmp file or a .dat file.
//	'ftype' - DAT if from a .dat file, TMP if from a .tmp file.
void ftype_table_print(TABLE *table, FILE_TYPE ftype){
	int i, init = (ftype == DAT ? table_root_size(table) : 0);
	char *filename = append_string((char *) table->name, (ftype == DAT ? ".dat" : ".tmp"));
	FILE *fp = fopen(filename, "r");
	void *record;

	free(filename);
	if(!fp) return;

	for(i = 0; ; i++){
		record = file_get_record(i, init, table->recordSize, fp);
		if(!record) break;
		record_print(record, table);
		free(record);
	}
	fclose(fp);
}

//Prints the header of a table:
//	<NAME>
//	<FIELD[0]>...<FIELD[N]>
void table_print_header(TABLE *table){
	int i;
	printf("Table name: %s\n", (char *) table->name);
	if(table->fieldCounter) printf("|  ");
	for(i = 0; i < table->fieldCounter; i++)
		printf("%s  |  ", (char *) table->fields[i]->name);
	printf("\n");	
}

void table_print_info(TABLE *table){
	char *type;
	int i;
	printf("Table name: %s\n", (char *) table->name);
	for(i = 0; i < table->fieldCounter; i++){
		type = type_to_string(table->fields[i]->fieldType);
		printf("\tField name: %s  |  Type: %s  |  Size: %d\n",
				(char *) table->fields[i]->name,
				type,
				table->fields[i]->dataSize);
		free(type);
	}
}

void alltables_add(char *tablename){
	FILE *fp = fopen(ALLTABLES_FILE, "r+");
	if(!fp) fp = fopen(ALLTABLES_FILE, "w");
	append_to_file(tablename, strlen(tablename)+1, fp);
	fclose(fp);
}

void allindexes_add(char *tablename, char *fieldname){
	FILE *fp = fopen(ALLINDEXES_FILE, "r+");
	if(!fp) fp = fopen(ALLINDEXES_FILE, "w");
	append_to_file(tablename, strlen(tablename)+1, fp);
	append_to_file(fieldname, strlen(fieldname)+1, fp);
	fclose(fp);
}
