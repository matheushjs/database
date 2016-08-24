/*  
    Copyright (C) 2016 Matheus H. J. Saldanha

    This file is part of Database.

    Database is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Database is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <table_kernel.h>
#include <table_types.h>
#include <stats.h>
#include <boolean.h>

#define ALLTABLES_FILE "alltables.txt"
#define ALLINDEXES_FILE "allindexes.txt"

void alltables_add(char *tableName){
	FILE *fp = fopen(ALLTABLES_FILE, "r+");
	if(!fp) fp = fopen(ALLTABLES_FILE, "w");
	append_to_file(tableName, strlen(tableName)+1, fp);
	fclose(fp);
}

void allindexes_add(char *tableName, char *fieldName){
	FILE *fp = fopen(ALLINDEXES_FILE, "r+");
	if(!fp) fp = fopen(ALLINDEXES_FILE, "w");
	append_to_file(tableName, strlen(tableName)+1, fp);
	append_to_file(fieldName, strlen(fieldName)+1, fp);
	fclose(fp);
}

//Given the name of a table, an array of fieldNames in any order,
//and an array of values in an order compliant with the fieldNames' order,
//insert these values to the .tmp file of the table.
//The insertion is done following the original order of the table's fields.
void table_insert(char *tableName, char **fieldNames, void **values){
	int i, j;
	char *fileName;
	FILE *fp;
	TABLE *table = table_from_file(tableName);
	TABLE_FIELD *curr_field;

	stats_inc(STATS_NINSERTS);

	fileName = append_string(tableName, ".tmp");
	fp = fopen(fileName, "r+");
	if(!fp) fp = fopen(fileName, "w+");

	for(i = 0; i < table->fieldCounter; i++){
		curr_field = table->fields[i];
		for(j = 0; j < table->fieldCounter; j++){
			if(strcmp(curr_field->name, fieldNames[j]) == 0){
				append_to_file(values[j], curr_field->dataSize, fp);
				break;
			}
		}
	}
	fclose(fp);
	free(fileName);
	table_destroy(&table);
}

//Creates an index for the table 'tableName', based on the field 'fieldName'.
//The index file '.idx' contains all values of field 'fieldName' and their offsets in the '.dat' file.
bool table_index(char *tableName, char *fieldName){
	int i, pos, offset, size, dat_size;
	char *dat_file, *idx_file, *data;
	FILE *dat_fp, *idx_fp;
	TABLE *table = table_from_file(tableName);

	if(!table) return FALSE;

	//Find the order in which the field appears.
	for(pos = 0; pos < table->fieldCounter; pos++)
		if(strcmp(table->fields[pos]->name, fieldName) == 0) break;
	if(pos == table->fieldCounter){
		table_destroy(&table);
		return FALSE;
	}
	
	//Moves all data from the '.tmp' file to the '.dat' file.
	tmp_to_dat(tableName);

	//Gets size of '.dat' file and put file cursor
	//after the initial data about the TABLE struct
	dat_file = append_string(tableName, ".dat");
	dat_fp = fopen(dat_file, "r");
	dat_size = file_size(dat_fp);
	free(dat_file);
	fseek(dat_fp, table->rootSize, SEEK_SET);

	//Creates the .idx file.
	idx_file = append_strings(4, tableName, "-", fieldName, ".idx");
	if(!file_exist(idx_file)) allindexes_add(tableName, fieldName);
	idx_fp = fopen(idx_file, "w+");
	free(idx_file);
	
	//Indexing procedure.
	while((offset = ftell(dat_fp)) < dat_size){
		for(i = 0; i < table->fieldCounter; i++){
			size = table->fields[i]->dataSize;
			if(i == pos){
				data = (char *) malloc(size);
				fread(data, size, 1, dat_fp);
				fwrite(data, size, 1, idx_fp);
				fwrite(&offset, sizeof(offset), 1, idx_fp);
				free(data);
			} else {
				fseek(dat_fp, size, SEEK_CUR);
			}
		}
	}

	table_destroy(&table);
	fclose(dat_fp);
	fclose(idx_fp);
	return TRUE;
}

//Sorts a .idx file for the table 'tableName' that was indexed by field 'fieldName'.
bool table_index_sort(char *tableName, char *fieldName){
	int recordSize, nrecords, i, j;
	char *idx_file;
	void *curr, *aux;
	FILE *fp;
	TABLE_FIELD *field = field_from_file(tableName, fieldName);

	if(!field) return FALSE;

	recordSize = field->dataSize + sizeof(int);

	//Opens the .idx file
	idx_file = append_strings(4, tableName, "-", fieldName, ".idx");
	fp = fopen(idx_file, "r+");
	free(idx_file);

	nrecords = file_size(fp)/recordSize;

	//Create containers and start insertion-sorting procedure.
	curr = malloc(recordSize);
	aux = malloc(recordSize);
	for(i = 1; i < nrecords; i++){
		file_get_record_ovl(curr, (j = i), 0, recordSize, fp);
		while(--j >= 0){
			file_get_record_ovl(aux, j, 0, recordSize, fp);
			if(type_higher(aux, curr, field) || //ensures lower records first.
			  (type_equal(aux, curr, field) &&  //ensures earlier records first.
			   *(int *)(aux+field->dataSize) > *(int *)(curr+field->dataSize))){
				file_save_record(aux, j+1, 0, recordSize, fp);
			} else break;
		}
		file_save_record(curr, j+1, 0, recordSize, fp);
	}
	
	free(aux);
	free(curr);
	free(field);
	fclose(fp);
	return TRUE;
}

//Prints a record from the table 'table'.
void record_print(void *record, TABLE *table){
	int i, pos = 0;
	for(i = 0; i < table->fieldCounter; i++){
		type_value_print(record+pos, table->fields[i]);
		pos += table->fields[i]->dataSize;
		if(i != table->fieldCounter-1) printf(", ");
	}
	printf("\n");
}

//Select procedure upon the .idx file.
//Returns the number of matched records.
int idx_select(TABLE *table, TABLE_FIELD *field, FILE *idx_fp, FILE *dat_fp, void *value){
	int lo, hi, cur, offset, nrecords, idx_rSize, nfound = 0;
	void *data, *record;
	
	idx_rSize = field->dataSize + sizeof(int);
	nrecords = file_size(idx_fp) / idx_rSize;
	if(!nrecords) return 0;

	hi = nrecords-1;
	lo = 0;
	for(;;){
		cur = (hi + lo) / 2;
		data = file_get_record(cur, 0, idx_rSize, idx_fp);
		
		if(type_equal(value, data, field)){
			free(data);

			//Finds the first occurence of the data.
			while(--cur >= 0){
				data = file_get_record(cur, 0, idx_rSize, idx_fp);
				if(!type_equal(value, data, field)){ free(data); break; }
				free(data);
			};
			
			//Prints all data from first occurence to last.
			record = malloc(table->recordSize);
			while(++cur < nrecords){
				//Fetches record on .idx file, which is [value] + [offset of value]
				data = file_get_record(cur, 0, idx_rSize, idx_fp);
				
				if(type_equal(value, data, field)){
					nfound++;
					
					//Store [offset of value]
					offset = *(int *) (data + field->dataSize);
					
					//Reads record from .dat file, corresponding to [offset of value]
					fseek(dat_fp, offset, SEEK_SET);
					fread(record, table->recordSize, 1, dat_fp);
					
					//Prints
					record_print(record, table);
				} else{ free(data); break; }
				free(data);
			}

			free(record);
			return nfound;
		} else if(type_higher(value, data, field)){
			lo = cur+1;
		} else{
			hi = cur-1;
		}
		free(data);
		if(lo > hi) return nfound;
	}
}

//Select procedure upon .dat and .tmp files.
//Returns the number of matched records.
int file_select(TABLE *table, TABLE_FIELD *field, FILE *fp, int init, void *value){
	int i, offset, field_init, nrecords, count = 0;	
	void *data = malloc(table->recordSize);

	field_init = field_offset(table, (char *) field->name);
	nrecords = (file_size(fp) - init) / table->recordSize;
	for(i = 0; i < nrecords; i++){
		offset = init + i*table->recordSize;
		fseek(fp, offset+field_init, SEEK_SET);
		fread(data, field->dataSize, 1, fp);
		if(type_equal(data, value, field)){
			fseek(fp, offset, SEEK_SET);
			fread(data, table->recordSize, 1, fp);
			count++;
			record_print(data, table);
		}
	}

	free(data);
	return count;
}

//Applies 'select' procedure upon table 'tableName'.
//Returns number of records found.
int table_select_records(char *tableName, char *fieldName, char *value){
	int bin_count = 0, seq_count = 0;
	char *idx_file, *dat_file, *tmp_file;
	FILE *fp, *dat_fp;

	stats_inc(STATS_NSELECTS);

	//Prepare table and field.
	TABLE *table = table_from_file(tableName);
	TABLE_FIELD *field = field_from_file(tableName, fieldName);
	if(!table || !field){
		table_destroy(&table);
		free(field);
		return -1;
	}

	//Search .dat or .idx files.
	idx_file = append_strings(4, tableName, "-", fieldName, ".idx");
	dat_file = append_string(tableName, ".dat");
	fp = fopen(idx_file, "r");
	dat_fp = fopen(dat_file, "r");
	free(idx_file);
	free(dat_file);
	if(fp){
		bin_count = idx_select(table, field, fp, dat_fp, value);
		fclose(fp);
	} else {
		seq_count = file_select(table, field, dat_fp, table->rootSize, value);
	}

	tmp_file = append_string(tableName, ".tmp");
	fp = fopen(tmp_file, "r");
	free(tmp_file);
	if(fp) seq_count += file_select(table, field, fp, 0, value);
	if(!(seq_count|bin_count)) printf("null\n");

	stats_set(seq_count, STATS_NLSR);
	stats_set(bin_count, STATS_NLBR);

	free(field);
	fclose(fp);
	fclose(dat_fp);
	table_destroy(&table);
	return seq_count + bin_count;
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

char *type_to_string(FIELD_TYPE ftype){
	char *result = malloc(10);
	strcpy(	result,
		ftype == STRING ? "char" :
		ftype == CHAR ? "char" :
		ftype == INT ? "int" :
		ftype == FLOAT ? "float" :
		ftype == DOUBLE ? "double" : "");
	return result;
}

void table_print_info(TABLE *table){
	char *type;
	int i;
	printf("\nTablename: %s\n", (char *) table->name);
	for(i = 0; i < table->fieldCounter; i++){
		type = type_to_string(table->fields[i]->fieldType);
		printf("\tField: %s Type: %s Size %d\n",
				(char *) table->fields[i]->name,
				type,
				table->fields[i]->dataSize - (table->fields[i]->fieldType == STRING ? 1 : 0));
				//subtracts 1 from dataSize (disregards the '\0') if the type is a STRING.
		free(type);
	}
	printf("\n");
}

//Prints a table from either a .tmp file or a .dat file.
//	'ftype' - DAT if from a .dat file, TMP if from a .tmp file.
void ftype_table_print(TABLE *table, FILE_TYPE ftype){
	int i, init = (ftype == DAT ? table_root_size(table) : 0);
	char *fileName = append_string((char *) table->name, (ftype == DAT ? ".dat" : ".tmp"));
	FILE *fp = fopen(fileName, "r");
	void *record;

	free(fileName);
	if(!fp) return;

	for(i = 0; ; i++){
		record = file_get_record(i, init, table->recordSize, fp);
		if(!record) break;
		record_print(record, table);
		free(record);
	}
	fclose(fp);
}

//Prints all records from a table, from .dat and .tmp files.
void table_print(char *tableName){
	TABLE *table = table_from_file(tableName);

	table_print_header(table);
	ftype_table_print(table, DAT);
	ftype_table_print(table, TMP);

	table_destroy(&table);
}

//Prints information about all tables in current workspace.
void alltables_print(){
	FILE *fp = fopen(ALLTABLES_FILE, "r");
	char tableName[MAX_NAME_SIZE];
	int i;
	TABLE *table;
	
	stats_inc(STATS_NSAT);	//nShow All Tables

	if(!fp){
		printf("There are no tables.");
		return;
	}
	for(;;){
		if(file_EOF(fp)) break;
		for(i=0; (tableName[i] = fgetc(fp)) != '\0'; i++);
		table = table_from_file(tableName);
		table_print_info(table);
		table_destroy(&table);
	}
	fclose(fp);
}

//Prints information about all indexes in current workspace.
void allindexes_print(){
	FILE *fp = fopen(ALLINDEXES_FILE, "r");
	char string[MAX_NAME_SIZE];
	int i;
	
	stats_inc(STATS_NSAI);	//nShow All Indexes

	if(!fp){
		printf("There are no indexes.");
		return;
	}
	for(;;){
		if(file_EOF(fp)) break;
		printf("\nIndex information\n");
		for(i = 0; (string[i] = fgetc(fp)) != '\0'; i++);
		printf("\tTablename: %s\n", string);
		for(i = 0; (string[i] = fgetc(fp)) != '\0'; i++);
		printf("\tFieldname: %s\n\n", string);
	}
	fclose(fp);
}
