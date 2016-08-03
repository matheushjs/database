#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

char **match(char *string, char *PATTERN, int nvars) {
	regex_t re;
	int i, start, len, counter = 0;
	regmatch_t *rm;
	char **allstrings = NULL;

	if(regcomp(&re, PATTERN, REG_EXTENDED) != 0) {
		fprintf(stderr, "Failed to compile regex '%s'\n", PATTERN);
		return allstrings;
	}

	rm = (regmatch_t *) malloc(sizeof(regmatch_t) * nvars);
	if(regexec(&re, string, nvars, rm, 0) == 0) {
		for(i = 0; i < nvars; i++) {
			start = rm[i].rm_so;
			len = rm[i].rm_eo - rm[i].rm_so;
			//printf("Start: %d End: %d Length: %d\n", 
			//			rm[i].rm_so, rm[i].rm_eo, len);

			allstrings = (char **) realloc(allstrings, sizeof(char *) * (counter+1));
			allstrings[counter] = (char *) malloc(sizeof(char)*(len+1));
			memcpy(allstrings[counter], string+start, len);
			allstrings[counter][len] = '\0';
			counter++;
		}
	} 

	free(rm);
	regfree(&re);
	return allstrings;
}
