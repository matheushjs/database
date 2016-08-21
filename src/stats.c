#include <stdio.h>
#include <string.h>
#include <stats.h>

static int stats[STATS_ENUMCOUNT];

void stats_reset(){
	memset(stats, 0, sizeof(stats));
}

void stats_inc(enum STATS_T s){
	stats[s]++;
}

void stats_set(int value, enum STATS_T s){
	stats[s] = value;
}

void stats_print(){
	printf("#Tables: %d\n", stats[STATS_NTABLES]);
	printf("#Indexes: %d\n", stats[STATS_NINDEXES]);
	printf("#Inserts: %d\n", stats[STATS_NINSERTS]);
	printf("#Selects: %d\n", stats[STATS_NSELECTS]);
	printf("#Sorts: %d\n", stats[STATS_NSORTS]);
	printf("#ShowAllTables: %d\n", stats[STATS_NSAT]);
	printf("#ShowAllIndexes: %d\n", stats[STATS_NSAI]);
	printf("#Records in last select (binary search): %d\n", stats[STATS_NLBR]);
	printf("#Records in last select (sequential search): %d\n", stats[STATS_NLSR]);
}
