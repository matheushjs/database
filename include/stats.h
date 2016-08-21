#ifndef _STATS_H
#define _STATS_H

enum STATS_T {
	STATS_NTABLES,
	STATS_NINDEXES,
	STATS_NINSERTS,
	STATS_NSELECTS,
	STATS_NSORTS,
	STATS_NSAT,	//Show All Tables
	STATS_NSAI,	//Show All Indexes
	STATS_NLBR,	//Last Binary Records
	STATS_NLSR,	//Last Sequential Records
	STATS_ENUMCOUNT
};

void stats_reset();
void stats_inc(enum STATS_T);
void stats_set(int, enum STATS_T);
void stats_print();

#endif
