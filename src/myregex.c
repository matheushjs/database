#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <boolean.h>

char **reg_parse(char *string, char *pattern, int nvars) {
	int i, start, len, counter = 0;
	char **allstrings = NULL;
	regmatch_t *rm;
	regex_t re;

	//Currently ignores letter case.
	if(regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) {
		fprintf(stderr, "Failed to compile regex pattern '%s'\n", pattern);
		return allstrings;
	}

	rm = (regmatch_t *) malloc(sizeof(regmatch_t) * nvars);
	if(regexec(&re, string, nvars, rm, 0) == 0) {
		for(i = 0; i < nvars; i++) {
			start = rm[i].rm_so;
			len = rm[i].rm_eo - rm[i].rm_so;

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

//Returns TRUE if 'string' matches pattern.
bool reg_match(char *string, char *pattern){
	bool result = FALSE;
	regmatch_t rm;
	regex_t re;

	//printf("Matching {%s} with {%s}\n", string, pattern);

	//Ignores letter case.
	if(regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) {
		fprintf(stderr, "Failed to compile regex pattern '%s'\n", pattern);
		return FALSE;
	}

	if(regexec(&re, string, 1, &rm, 0) == 0) result = TRUE;
	//printf(result ? "Success\n" : "Failure\n");
	regfree(&re);
	return result;
}
