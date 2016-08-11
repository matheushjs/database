#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <stdarg.h>
#include <boolean.h>
#include <errno.h>

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

//Appends multiple strings.
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
	} while (c != '\n');
	line[counter-1] = '\0';

	return line;
}

//Given a string s, return all substrings delimited by 'sep' as an array of strings.
//Returns NULL if the string given has length 0.
char **split_string(char *s, const char sep, int *count){
	int i, start, len = strlen(s);
	char **res = NULL;
	
	*count = 0;
	for(i = 0; i < len; i++){
		(*count)++;
		res = (char **) realloc(res, sizeof(char *) * (*count));
		start = i;
		while(s[i] != '\0' && s[i] != sep) i++;
		res[*count-1] = strndup(s+start, i-start);
	}
	return res;
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
//	of records of a fixed size 'recordSize'.
//This function returns the index-th record from such sequence.
//	index		- The index of the record to be retrieved.
//	initOffset	- Offset for the beginning of the sequence of records.
//	recordSize 	- Size, in bytes, of each record on the sequence.
void *file_get_record(int index, int initOffset, int recordSize, FILE *fp){
	void *record = malloc(recordSize);
	fseek(fp, initOffset + recordSize*index, SEEK_SET);
	if(fread(record, recordSize, 1, fp) != 1){ free(record); return NULL; }
	return record;
}

//Turns every alphabetical character in 's' to lowercase.
//Returns the resulting string in another region of memory.
char *lowercase(char *s){
	int i, size = strlen(s);
	char *r = malloc(sizeof(char) * size);
	for(i = 0; i < size; i++)
		r[i] = (s[i] > 64 && s[i] < 91) ? s[i] + 32 : s[i];
	return r;
}

//Turns every alphabetical character in 's' to lowercase.
void to_lowercase(char *s){
	for(; *s != '\0'; s++)
		*s = (*s > 64 && *s < 91) ? *s + 32 : *s;
}

//Ignore-case string comparison.
bool icase_strcmp(char *s, char *t){
	char *lower1, *lower2;
	int len = strlen(s);
	bool res = FALSE;
	if(len != strlen(t)) return FALSE;
	lower1 = lowercase(s);
	lower2 = lowercase(t);
	if(memcmp(lower1, lower2, len) == 0) res = TRUE;
	free(lower1), free(lower2);
	return res;
}

//Terminates program after printing error messages.
void die(const char *message){
	fprintf(stderr, message);
	perror("ERROR:");
	exit(0);
}
