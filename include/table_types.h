#ifndef _TABLE_TYPES_H
#define _TABLE_TYPES_H

#define MAX_NAME_SIZE 51

typedef enum {
	DAT,
	TMP,
	IDX
} FILE_TYPE;

typedef enum {
	INT,
	FLOAT,
	DOUBLE,
	CHAR,
	STRING
} FIELD_TYPE;

typedef struct {
	char name[MAX_NAME_SIZE];
	FIELD_TYPE fieldType;
	int dataSize;
} TABLE_FIELD;

typedef struct {
	char name[MAX_NAME_SIZE];
	TABLE_FIELD **fields;
	int fieldCounter;
	int rootSize;
	int recordSize;
} TABLE;

#endif
