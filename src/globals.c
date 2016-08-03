#include <stdio.h>
#include <string.h>
#include <globals.h>

struct STATS stats;

void stats_set(){
	memset(&stats, 0, sizeof(stats));
}

void stats_print(){
	printf("#Tables: %d\n", stats.nTables);
	printf("#Indexes: %d\n", stats.nIndexes);
	printf("#Inserts: %d\n", stats.nInserts);
	printf("#Selects: %d\n", stats.nSelects);
	printf("#Sorts: %d\n", stats.nSorts);
	printf("#ShowAllTables: %d\n", stats.nSAT);
	printf("#ShowAllIndexes: %d\n", stats.nSAI);
	printf("#Records in last select (binary search): %d\n", stats.nLBR);
	printf("#Records in last select (sequential search): %d\n", stats.nLSR);
}
