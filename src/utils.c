#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <stdarg.h>
#include <globals.h>

//Given a 'src' string
//Creates a string that equals 'src' with 'append' appended to the end of it.
//e.g.:
//	append_string("name", ".dat");
//	returns "name.dat"
char *append_string(const char *src, const char *append){
	int src_size = strlen(src), app_size = strlen(append);
	char *filename = (char *) malloc(sizeof(char) * (src_size+app_size+1));
	
	memcpy(filename, src, src_size);
	memcpy(&filename[src_size], append, app_size+1);
	return filename;
}

char *append_strings(int nstrings, ...){
	int i, index = 0, len;
	char *result = NULL, *curr;
	va_list strings;

	va_start(strings, nstrings);
	for(i = 0; i < nstrings; i++){
		curr = va_arg(strings, char *);
		len = strlen(curr);
		result = (char *) realloc(result, sizeof(char) * (len+index));

		memcpy(result+index, curr, len);
		index += len;
	}
	result = (char *) realloc(result, sizeof(char) * (index+1));
	result[index] = '\0';

	va_end(strings);

	return result;
}

char *read_line(FILE *stream) {
	char *line = NULL;
	int counter = 0;
	char c;

	do {
		c = fgetc(stream);
		line = (char *) realloc(line, sizeof(char) * (counter+1));
		line[counter++] = c;
	} while (c != DELIMITER);

	line = (char *) realloc(line, sizeof(char) * (counter+1));
	line[counter++] = '\0';

	return line;
}

void matrix_free(void **m, int row){
	if(!m) return;
	for(; row; free(m[--row]));
	free(m);
}

//Appends 'size' bytes of 'value' into file pointed to by 'fp'.
void append_to_file(void *value, int size, FILE *fp){
	fseek(fp, 0, SEEK_END);
	fwrite(value, size, 1, fp);
}

void append_files(char *dest, char *src){
	int size;
	void *data;
	FILE *fp;
	
	//Get size of src file.
	fp = fopen(src, "r");
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	data = malloc(size);

	//Read data from src.
	fseek(fp, 0, SEEK_SET);
	fread(data, size, 1, fp);
	fclose(fp);

	//Append data to dest.
	fp = fopen(dest, "r+");
	fseek(fp, 0, SEEK_END);
	fwrite(data, size, 1, fp);
	fclose(fp);
	free(data);
}

int get_file_size(FILE *fp){
	int result;
	fseek(fp, 0, SEEK_END);
	result = (int) ftell(fp);
	return result;
}

bool file_exist(char *filename){
	FILE *fp = fopen(filename, "r");
	if(!fp) return FALSE;
	fclose(fp);
	return TRUE;
}

bool file_EOF(FILE *fp){
	char c = fgetc(fp);
	if(c == EOF) return TRUE;
	fseek(fp, -1, SEEK_CUR);
	return FALSE;
}

//Beginning on position initOffset, the file pointed to by 'fp' has a sequence
//of records of a fixed size 'recordSize'.
//This function returns the index-th record from such sequence.
//	index		- The index of the record to be retrieved.
//	initOffset	- Offset for the beginning of the sequence.
//	lineSize 	- Size, in bytes, of each line of the sequence.
void *file_get_record(int index, int initOffset, int recordSize, FILE *fp){
	void *record = malloc(recordSize);
	fseek(fp, initOffset + recordSize*index, SEEK_SET);
	if(fread(record, recordSize, 1, fp) != 1){free(record); return NULL;}
	return record;
}
