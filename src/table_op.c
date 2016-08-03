#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <table_kernel.h>
#include <myregex.h>
#include <globals.h>

#include <assert.h> //For debugging

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

//Saves a table to a .dat file
void table_to_file(TABLE *table){
	char *filename;
	FILE *fp;
	int i;

	filename = append_string(table->name, ".dat");
	if(!file_exist(filename)){
		alltables_add(table->name);
		stats.nTables++;
	}
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

//Given the name of a table,
//an array of fieldnames in a certain order,
//and an array of values in the same convenient order,
//insert these values to the .tmp file of the table.
//e.g
//	tablename = "people" with fields {name, age, ID}
//	fieldnames = {"name", "ID", "age"}
//	values = {"Matheus", "162", "20"}
//adds value "Matheus" to field "name" of table "people"
//     value "20" to field "age"
//     value "162" to field "ID"
//Note the order in which the values were added to the table.
void table_insert(char *tablename, char **fieldnames, char **values){
	int i, j;
	TABLE *table = table_from_file(tablename);
	TABLE_FIELD *curr_field;
	char *filename;
	FILE *fp;

	stats.nInserts++;

	filename = append_string(tablename, ".tmp");
	fp = fopen(filename, "r+");
	if(!fp) fp = fopen(filename, "w+");
	assert(fp);

	for(i = 0; i < table->fieldCounter; i++){
		curr_field = table->fields[i];
		for(j = 0; j < table->fieldCounter; j++){
			if(strcmp(curr_field->name, fieldnames[j]) == 0){
				type_append_from_string(values[j], curr_field, fp);
				break;
			}
		}
	}
	fclose(fp);
	free(filename);
	table_destroy(&table);
}

//Creates an index for the table 'tablename', based on the field 'fieldname'.
//The index file '.idx' contains all values of field 'fieldname' and their positions in the '.dat' file.
void table_index(char *tablename, char *fieldname){
	int i, pos, aux, size, dat_size;
	char *dat_file, *idx_file, *data;
	FILE *dat_fp, *idx_fp;
	TABLE *table = table_from_file(tablename);

	//Moves all data from the '.tmp' file to the '.dat' file.
	tmp_to_dat(tablename);

	//Gets size of '.dat' file and put file cursor
	//after the initial data about the TABLE struct
	dat_file = append_string(tablename, ".dat");
	dat_fp = fopen(dat_file, "r");
	dat_size = get_file_size(dat_fp);
	free(dat_file);
	fseek(dat_fp, table->rootSize, SEEK_SET);

	//Find the order in which the field appear.
	for(pos = 0; pos < table->fieldCounter; pos++)
		if(strcmp(table->fields[pos]->name, fieldname) == 0) break;

	//Creates the .idx file.
	idx_file = append_strings(4, tablename, "-", fieldname, ".idx");
	if(!file_exist(idx_file)) allindexes_add(tablename, fieldname);
	idx_fp = fopen(idx_file, "w+");
	free(idx_file);
	
	//Indexing procedure.
	while(ftell(dat_fp) < dat_size){
		aux = ftell(dat_fp);
		for(i = 0; i < table->fieldCounter; i++){
			size = table->fields[i]->dataSize;
			if(i == pos){
				data = (char *) malloc(size);
				fread(data, size, 1, dat_fp);
				fwrite(data, size, 1, idx_fp);
				fwrite(&aux, sizeof(aux), 1, idx_fp);
				free(data);
			} else{
				fseek(dat_fp, size, SEEK_CUR);
			}
		}
	}

	table_destroy(&table);
	fclose(dat_fp);
	fclose(idx_fp);
}

//Sorts a .idx file for the table 'tablename' that was indexed by field 'fieldname'.
void index_sort(char *tablename, char *fieldname){
	FILE *fp;
	char *idx_file;
	void *cont1, *cont2;
	int recordSize, nrecords, i, j;
	TABLE_FIELD *field = field_from_file(tablename, fieldname);
	
	recordSize = field->dataSize + sizeof(int);

	//Opens the .idx file
	idx_file = append_strings(4, tablename, "-", fieldname, ".idx");
	fp = fopen(idx_file, "r+");
	free(idx_file);

	nrecords = get_file_size(fp)/recordSize;

	//Create containers and start sorting procedure.
	cont1 = malloc(recordSize);
	cont2 = malloc(recordSize);
	for(i = 0; i < nrecords - 1; i++){
		fseek(fp, recordSize*i, SEEK_SET);
		fread(cont1, recordSize, 1, fp);
		for(j = i + 1; j < nrecords; j++){
			fseek(fp, recordSize*j, SEEK_SET);
			fread(cont2, recordSize, 1, fp);
			if(type_higher(cont1, cont2, field) ||   //lower records first.
			   (type_equal(cont1, cont2, field) &&   //newer records first (lower offset in .dat file).
			   *(int *)(cont1+field->dataSize) > *(int *)(cont2+field->dataSize))){
				fseek(fp, recordSize*i, SEEK_SET);
				fwrite(cont2, recordSize, 1, fp);
				fseek(fp, recordSize*j, SEEK_SET);
				fwrite(cont1, recordSize, 1, fp);
				
				//Updates cont1 with the switched value
				memcpy(cont1, cont2, recordSize);
			}
		}
	}

	free(cont1);
	free(cont2);
	free(field);
	fclose(fp);
}

//Prints all records from a table, from .dat and .tmp files.
void table_print(char *tablename){
	TABLE *table = table_from_file(tablename);

	table_print_header(table);
	ftype_table_print(table, DAT);
	ftype_table_print(table, TMP);

	table_destroy(&table);
}

//Select procedure upon the .idx file.
//Searches using binary search.
void idx_select(TABLE *table, TABLE_FIELD *field, FILE *idx_fp, FILE *dat_fp, void *value){
	int lo, hi, cur, offset, nrecords, idx_rSize;
	void *data, *record;

	stats.nLBR = 0;

	idx_rSize = field->dataSize + sizeof(int);
	nrecords = get_file_size(idx_fp) / idx_rSize;

	//It makes sense to place a '-1' here, but that will make the binary search fail.
	//That's due to approximation issues.
	hi = nrecords;
	lo = 0;

	if(!hi) return;
	
	for(;;){
		cur = (hi + lo) / 2;
		data = file_get_record(cur, 0, idx_rSize, idx_fp);
		
		if(type_higher(value, data, field)){
			free(data);
			if(cur == lo) return;
			lo = cur;
		} else if(type_equal(value, data, field)){
			free(data);

			//Finds the first occurance of the data.
			while(--cur >= 0){
				data = file_get_record(cur, 0, idx_rSize, idx_fp);
				if(!type_equal(value, data, field)){ free(data); break; }
				free(data);
			};
			
			//Prints all data
			record = malloc(table->recordSize);
			while(++cur < nrecords){
				data = file_get_record(cur, 0, idx_rSize, idx_fp);
				if(type_equal(value, data, field)){
					offset = *(int *) (data + field->dataSize);
					fseek(dat_fp, offset, SEEK_SET);
					fread(record, table->recordSize, 1, dat_fp);
					stats.nLBR++;
					record_print(record, table);
				} else{ free(data); break; }
				free(data);
			}

			free(record);
			return;
		} else {
			free(data);
			if(cur == lo) return;
			hi = cur;
		}
	}
}

void file_select(TABLE *table, TABLE_FIELD *field, FILE *fp, int init, void *value_bytes){
	int i, offset, field_init, nrecords;	
	void *data = malloc(table->recordSize);

	stats.nLSR = 0;

	field_init = field_offset(table, (char *) field->name);
	nrecords = (get_file_size(fp) - init) / table->recordSize;

	for(i = 0; i < nrecords; i++){
		offset = init + i*table->recordSize;
		fseek(fp, offset+field_init, SEEK_SET);
		fread(data, field->dataSize, 1, fp);
		if(type_equal(data, value_bytes, field)){
			fseek(fp, offset, SEEK_SET);
			fread(data, table->recordSize, 1, fp);
			stats.nLSR++;
			record_print(data, table);
		}
	}

	free(data);
}

void select_records(char *tablename, char *fieldname, char *value){
	char *idx_file, *dat_file, *tmp_file;

	stats.nSelects++;

	//Prepare file names.
	idx_file = append_strings(4, tablename, "-", fieldname, ".idx");
	dat_file = append_string(tablename, ".dat");
	tmp_file = append_string(tablename, ".tmp");

	//Prepare table and field.
	TABLE *table = table_from_file(tablename);
	TABLE_FIELD *field = field_from_file(tablename, fieldname);

	//Search .dat file.
	FILE *fp, *dat_fp;
	void *value_bytes = type_data_from_string(value, field);

	fp = fopen(idx_file, "r");
	dat_fp = fopen(dat_file, "r");
	free(idx_file);
	free(dat_file);
	if(fp != NULL){
		idx_select(table, field, fp, dat_fp, value_bytes);
		fclose(fp);
	} else {
		file_select(table, field, dat_fp, table->rootSize, value_bytes);
	}

	fp = fopen(tmp_file, "r");
	free(tmp_file);
	if(fp) file_select(table, field, fp, 0, value_bytes);

	free(value_bytes);
	free(field);
	fclose(fp);
	fclose(dat_fp);
	table_destroy(&table);
}

void alltables_print(){
	FILE *fp = fopen(ALLTABLES_FILE, "r");
	
	stats.nSAT++;

	if(!fp){
		printf("There are no tables.");
		return;
	}

	char tablename[MAX_NAME_SIZE];
	int i;
	TABLE *table;
	for(;;){
		if(file_EOF(fp)) break;;
		for(i=0; (tablename[i] = fgetc(fp)) != '\0'; i++);
		table = table_from_file(tablename);
		table_print_info(table);
		table_destroy(&table);
	}
	fclose(fp);
}

void allindexes_print(){
	FILE *fp = fopen(ALLINDEXES_FILE, "r");
	
	stats.nSAI++;

	if(!fp){
		printf("There are no indexes.");
		return;
	}

	char string[MAX_NAME_SIZE];
	int i;
	printf("Index Information");
	for(;;){
		if(file_EOF(fp)) break;
		for(i = 0; (string[i] = fgetc(fp)) != '\0'; i++);
		printf("\n\tTablename: %s\n", string);
		for(i = 0; (string[i] = fgetc(fp)) != '\0'; i++);
		printf("\tFieldname: %s\n", string);
	}
	fclose(fp);
}

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
