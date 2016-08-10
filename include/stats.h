#ifndef _STATS_H
#define _STATS_H

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

#endif
