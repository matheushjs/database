#ifndef _GLOBALS_H
#define _GLOBALS_H

#define MAX_NAME_SIZE 51
#define DELIMITER ';'
#define ALLTABLES_FILE "alltables.txt"
#define ALLINDEXES_FILE "allindexes.txt"

struct STATS {
	int nTables,
	    nIndexes,
	    nInserts,
	    nSelects,
	    nSorts,
	    nSAT,	//Show All Tables
	    nSAI,	//Show All Indexes
	    nLBR,	//Last Binary Records
	    nLSR;	//Last Sequential Records
};

extern struct STATS stats;
void stats_set();
void stats_print();

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

typedef enum {
	FALSE,
	TRUE
} bool;

#endif
