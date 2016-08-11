#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <boolean.h>

char **reg_parse(char *string, char *pattern, int nvars) {
	int i, start, size, counter = 0;
	char **regStrings = NULL;
	regmatch_t *rm;
	regex_t re;

	//Currently ignores letter case.
	if(regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) {
		fprintf(stderr, "Failed to compile regex pattern '%s'\n", pattern);
		return NULL;
	}

	rm = (regmatch_t *) malloc(sizeof(regmatch_t) * nvars);
	if(regexec(&re, string, nvars, rm, 0) == 0) {
		for(i = 0; i < nvars; i++) {
			start = rm[i].rm_so;
			size = rm[i].rm_eo - rm[i].rm_so;

			regStrings = (char **) realloc(regStrings, sizeof(char *) * (counter+1));
			regStrings[counter] = (char *) malloc(sizeof(char) * (size+1));
			memcpy(regStrings[counter], string+start, size);
			regStrings[counter][size] = '\0';
			counter++;
		}
	} 

	free(rm);
	regfree(&re);
	return regStrings;
}

//Returns TRUE if 'string' matches pattern.
bool reg_match(char *string, char *pattern){
	bool result = FALSE;
	regmatch_t rm;
	regex_t re;

	//Ignores letter case.
	if(regcomp(&re, pattern, REG_EXTENDED | REG_ICASE) != 0) {
		fprintf(stderr, "Failed to compile regex pattern '%s'\n", pattern);
		return FALSE;
	}
	if(regexec(&re, string, 1, &rm, 0) == 0) result = TRUE;
	regfree(&re);
	return result;
}
