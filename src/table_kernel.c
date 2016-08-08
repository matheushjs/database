#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <table_types.h>
#include <globals.h>

#include <assert.h>

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

//Creates a table without any fields
TABLE *table_alloc(char *name){
	TABLE *table = (TABLE *) calloc(sizeof(TABLE), 1);
	int size = strlen(name) + 1;
	assert(size <= MAX_NAME_SIZE);	
	memcpy(table->name, name, size);
	table->rootSize = table_root_size_constant();
	return table;
}

//Deallocates a table from the memory
void table_destroy(TABLE **table){
	int i;
	if(!*table) return;

	for(i = 0; i < (*table)->fieldCounter; free((*table)->fields[i++]));
	free((*table)->fields);
	free(*table);
	*table = NULL;
}

//Adds a field to a table. 'dataSize' is the size of the data when it's saved into a file.
//Note that char[80] should have a dataSize of 81.
void table_add_field(TABLE *table, char *fieldname, FIELD_TYPE type, int dataSize){
	TABLE_FIELD *newfield = (TABLE_FIELD *) calloc(sizeof(TABLE_FIELD), 1);
	
	newfield->fieldType = type;
	newfield->dataSize = type == STRING ? dataSize :
			       type == INT ? sizeof(int) :
			       type == FLOAT ? sizeof(float) :
			       type == DOUBLE ? sizeof(double) :
			       type == CHAR ? sizeof(char) : -1;

	int size = strlen(fieldname) + 1;
	assert(size <= MAX_NAME_SIZE);
	memcpy(newfield->name, fieldname, size);

	table->fields = (TABLE_FIELD **)
		realloc(table->fields, sizeof(TABLE_FIELD *) * ((table->fieldCounter)+1));
	table->fields[table->fieldCounter++] = newfield;
	table->rootSize += sizeof(TABLE_FIELD);
	table->recordSize += newfield->dataSize;
}

//Saves initial table structure to a .dat file.
void table_to_file(TABLE *table){
	char *filename;
	FILE *fp;
	int i;

	filename = append_string(table->name, ".dat");
	fp = fopen(filename, "w+");
	
	//Saves data
	fwrite(&table->fieldCounter, sizeof(table->fieldCounter), 1, fp);
	fwrite(&table->rootSize, sizeof(table->rootSize), 1, fp);
	fwrite(&table->recordSize, sizeof(table->recordSize), 1, fp);
	for(i = 0; i < table->fieldCounter; i++)
		fwrite(table->fields[i], sizeof(TABLE_FIELD), 1, fp);
	free(filename);
	fclose(fp);
}

//Creates a table with:
//	name 'tablename'
//	number of fields 'nfields'
//	field[i] with name 'names[i]', type 'types[i]' and datasize 'sizes[i]'.
void table_create(char *tablename, int nfields, char **names, FIELD_TYPE *types, int *sizes){
	int i;
	TABLE *table = table_alloc(tablename);
	for(i = 0; i < nfields; i++)
		table_add_field(table, names[i], types[i], sizes[i]);
	table_to_file(table);
	table_destroy(&table);
}

//Loads a table from a .dat file
TABLE *table_from_file(char *tablename){
	FILE *fp;
	char *filename;
	int i;
	TABLE *table = table_alloc(tablename);
	TABLE_FIELD *field;

	//Appends ".dat" to tablename into a new variable
	filename = append_string(tablename, ".dat");
	fp = fopen(filename, "r");
	if(!fp){ table_destroy(&table); return NULL; };

	//Read data
	fread(&table->fieldCounter, sizeof(table->fieldCounter), 1, fp);
	fread(&table->rootSize, sizeof(table->rootSize), 1, fp);
	fread(&table->recordSize, sizeof(table->recordSize), 1, fp);
	table->fields = (TABLE_FIELD **) malloc(sizeof(TABLE_FIELD *) * table->fieldCounter);
	for(i = 0; i < table->fieldCounter; i++){
		field = (TABLE_FIELD *) malloc(sizeof(TABLE_FIELD) * 1);
		fread(field, sizeof(TABLE_FIELD), 1, fp);
		table->fields[i] = field;
	}
	free(filename);
	fclose(fp);
	return table;
}

//Returns the TABLE_FIELD* of the field 'fieldname'.
TABLE_FIELD *field_from_file(char *tablename, char *fieldname){
	int i;
	TABLE *table = table_from_file(tablename);
	TABLE_FIELD *result = NULL;

	for(i = 0; i < table->fieldCounter; i++){
		if(strcmp(fieldname, table->fields[i]->name) == 0){
			result = (TABLE_FIELD *) malloc(sizeof(TABLE_FIELD));
			memcpy(result, table->fields[i], sizeof(TABLE_FIELD));
			break;
		}
	}
	
	table_destroy(&table);
	return result;
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
