#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <table_types.h>
#include <boolean.h>

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

//Returns the 'offset' of the data from the field 'fieldName',
//relative to the initial position of the whole record.
int field_offset(TABLE *table, char *fieldName){
	int i = 0, size = 0;
	while(strcmp(fieldName, (char *) table->fields[i]->name) != 0){
		size += table->fields[i++]->dataSize;
		if(i == table->fieldCounter) return -1;
	}
	return size;
}

//Creates a table without any fields.
TABLE *table_alloc(char *name){
	int size = strlen(name) + 1;
	TABLE *table; 
	if(size > MAX_NAME_SIZE) return NULL;
	table = (TABLE *) calloc(sizeof(TABLE), 1);
	memcpy(table->name, name, size);
	table->rootSize = table_root_size_constant();
	return table;
}

//Deallocates a table from the memory
void table_destroy(TABLE **table){
	if(!*table) return;
	matrix_free((void **) (*table)->fields, (*table)->fieldCounter);
	free(*table);
	*table = NULL;
}

//Adds a field to a table. 'dataSize' is the size of the data when it's saved into a file.
//Note that char[80] should have a dataSize of 81.
//Returns FALSE on failure (fieldName is too long).
bool table_add_field(TABLE *table, char *fieldName, FIELD_TYPE type, int dataSize){
	int nameSize = strlen(fieldName) + 1;
	TABLE_FIELD *newField;

	if(nameSize > MAX_NAME_SIZE) return FALSE;
		
	newField = (TABLE_FIELD *) calloc(sizeof(TABLE_FIELD), 1);
	newField->fieldType = type;
	memcpy(newField->name, fieldName, nameSize);
	newField->dataSize = type == STRING ? dataSize :
			       type == INT ? sizeof(int) :
			       type == FLOAT ? sizeof(float) :
			       type == DOUBLE ? sizeof(double) :
			       type == CHAR ? sizeof(char) : -1;
	
	table->fields = (TABLE_FIELD **) realloc(table->fields,
			sizeof(TABLE_FIELD *) * (table->fieldCounter+1));
	table->fields[table->fieldCounter++] = newField;
	table->rootSize += sizeof(TABLE_FIELD);
	table->recordSize += newField->dataSize;
	return TRUE;
}

//Saves initial table structure to a .dat file.
void table_to_file(TABLE *table){
	char *fileName;
	FILE *fp;
	int i;

	fileName = append_string(table->name, ".dat");
	fp = fopen(fileName, "w+");
	
	//Saves data
	fwrite(&table->fieldCounter, sizeof(table->fieldCounter), 1, fp);
	fwrite(&table->rootSize, sizeof(table->rootSize), 1, fp);
	fwrite(&table->recordSize, sizeof(table->recordSize), 1, fp);
	for(i = 0; i < table->fieldCounter; i++)
		fwrite(table->fields[i], sizeof(TABLE_FIELD), 1, fp);
	free(fileName);
	fclose(fp);
}

//Creates a table with:
//	name 'tableName'
//	number of fields 'nfields'
//	field[i] with name 'names[i]', type 'types[i]' and datasize 'sizes[i]'.
void table_create(char *tableName, int nfields, char **names, FIELD_TYPE *types, int *sizes){
	int i;
	TABLE *table = table_alloc(tableName);
	for(i = 0; i < nfields; i++)
		table_add_field(table, names[i], types[i], sizes[i]);
	table_to_file(table);
	table_destroy(&table);
}

//Loads a table from a .dat file
TABLE *table_from_file(char *tableName){
	FILE *fp;
	char *fileName;
	int i;
	TABLE *table;

	//Appends ".dat" to tableName into a new variable
	fileName = append_string(tableName, ".dat");
	fp = fopen(fileName, "r");
	free(fileName);
	if(!fp) return NULL;

	//Read data
	table = table_alloc(tableName);
	fread(&table->fieldCounter, sizeof(table->fieldCounter), 1, fp);
	fread(&table->rootSize, sizeof(table->rootSize), 1, fp);
	fread(&table->recordSize, sizeof(table->recordSize), 1, fp);
	table->fields = (TABLE_FIELD **) malloc(sizeof(TABLE_FIELD *) * table->fieldCounter);
	for(i = 0; i < table->fieldCounter; i++){
		table->fields[i] = (TABLE_FIELD *) malloc(sizeof(TABLE_FIELD) * 1);
		fread(table->fields[i], sizeof(TABLE_FIELD), 1, fp);
	}
	fclose(fp);
	return table;
}

//Returns the TABLE_FIELD* of the field 'fieldName'.
TABLE_FIELD *field_from_file(char *tableName, char *fieldName){
	int i;
	TABLE *table = table_from_file(tableName);
	TABLE_FIELD *result = NULL;

	for(i = 0; i < table->fieldCounter; i++){
		if(strcmp(fieldName, table->fields[i]->name) == 0){
			result = (TABLE_FIELD *) malloc(sizeof(TABLE_FIELD));
			memcpy(result, table->fields[i], sizeof(TABLE_FIELD));
			break;
		}
	}
	
	table_destroy(&table);
	return result;
}

//Appends data from 'tableName.tmp' to 'tableName.dat'
//then erases all data in the '.tmp' file.
void tmp_to_dat(char *tableName){
	char *dat, *tmp;
	dat = append_string(tableName, ".dat");
	tmp = append_string(tableName, ".tmp");
	append_files(dat, tmp);
	fclose(fopen(tmp, "w"));
	free(dat);
	free(tmp);
}

//Returns TRUE if value1 is higher than value2, considering both as having type field->fieldType.
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
//STRING comparison is case insensitive.
//CHAR comparison is case sensitive.
bool type_equal(void *value1, void *value2, TABLE_FIELD *field){
	if(field->fieldType == STRING)
		return icase_strcmp((char *) value1, (char *) value2);
	return memcmp(value1, value2, field->dataSize) == 0 ? TRUE : FALSE;
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
			printf("'%c'", *(char *) value);
			break;
		case STRING:
			printf("'%s'", (char *) value);
			break;
	}
}
