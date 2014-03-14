#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "command.h"
#include <dirent.h>
#include <regex.h>


bool isMatch(char *regularExp, char *stringToMatch);

void expandWildCard(char * processed, char * unprocessed) {
	//printf("processed ====== %s\n", processed);
	//printf("unprocessed ====== %s\n", unprocessed);
	if (unprocessed == NULL || strlen(unprocessed) == 0) {
		//printf("=== ======%s\n", processed);
		Command::_currentSimpleCommand->insertArgument(strdup(processed));
		return;
	}
	char *index = strchr(unprocessed, '/');
	char component[1024];

	if (index == NULL) {  // not '/' 
		strcpy(component, unprocessed);
	} else if (index == unprocessed) {  // first char is '/'
		expandWildCard("/", unprocessed + 1);
		return;
	} else {  // '/' found in the middle 
		//component = (char*)malloc(strlen(unprocessed));
		strncpy(component, unprocessed, index-unprocessed);
		component[index-unprocessed] = 0;
	}

	//printf("%s\n", component);
	char *containStar = strchr(component, '*');
	char *containQuestionMark = strchr(component, '?');
	// no wildcard
	if (containStar == NULL && containQuestionMark == NULL) {
		char new_processed[1024];
		strcpy(new_processed, processed);
		if (strlen(new_processed) > 0  && strcmp(processed, "/") != 0) {
			strcat(new_processed, "/");
		}
		strcat(new_processed, component);
		//expandWildCard(new_processed, unprocessed + strlen(component)+1);
		if (index == NULL) {
			expandWildCard(new_processed, NULL);
		} else {
			expandWildCard(new_processed, unprocessed + strlen(component)+1);
		}
	} else {  // contains wildcard
		// create regular expression
		char new_component[1024];
		int j = 0;
		for (int i = 0; i < strlen(component); i ++) {
			if (component[i] == '*') {
				new_component[j++] = '.';
				new_component[j++] = '*';
			} else if (component[i] == '?') {
				new_component[j++] = '.';
			} else if (component[i] == '.') {
				new_component[j++] = '\\';
				new_component[j++] = '.';
			} else {
				new_component[j++] = component[i];
			}
		}
		new_component[j++] = 0;
		//printf("new_component ==== %s\n", new_component);
		//printf("component ==== %s\n", component);
		DIR *dirp;
		struct dirent *dp;
		char* arr[10240];
		int number_dirs = 0;

		//printf("processed ==== >>>>%s\n", processed);
		if (strlen(processed) > 0) {
			if ((dirp = opendir(processed)) == NULL) {
	        	return;
	   		}
   		} else {
   			if ((dirp = opendir(".")) == NULL) {
	        	return;
	   		}
   		}
    	do {
        	if ((dp = readdir(dirp)) != NULL) {
        		if (isMatch(new_component, dp->d_name)) {
        			if (dp->d_name[0] == '.' && component[0] == '.') {
        				arr[number_dirs ++] = strdup(dp->d_name);
        			} else if (dp->d_name[0] != '.') {
        				arr[number_dirs ++] = strdup(dp->d_name);
        			}
        			//printf("%s\n", dp->d_name);
        		} else {

        		}
        	}
    	} while (dp != NULL);
    	
    	// sort dirs
    	for (int i = 0; i < number_dirs; i ++) {
    		for (int j = i+1; j < number_dirs; j ++) {
    			if (strcmp(arr[i], arr[j]) > 0) {
    				char *tmp = arr[i];
    				arr[i] = arr[j];
    				arr[j] = tmp;
    			}
    		}
    	}
    	for (int i = 0; i < number_dirs; i ++) {
    		char new_processed[1024];
			strcpy(new_processed, processed);
			if (strlen(new_processed) > 0 && strcmp(processed, "/") != 0) {
				strcat(new_processed, "/");
			}
			strcat(new_processed, arr[i]);
			if (index == NULL) {
				expandWildCard(new_processed, NULL);
			} else {
				expandWildCard(new_processed, unprocessed + strlen(component)+1);
			}
			free(arr[i]);
    	}

	}




}

bool isMatch(char *regularExp, char *stringToMatch) {
	char regExpComplete[ 1024 ];
	sprintf(regExpComplete, "^%s$", regularExp );
	regex_t re;	
	int result = regcomp( &re, regExpComplete,  REG_EXTENDED|REG_NOSUB);
	if( result != 0 ) {
		return false;
    }

	regmatch_t match;
	result = regexec( &re, stringToMatch, 1, &match, 0 );

	const char * matchResult = "MATCHES";
	if ( result != 0 ) {
		//matchResult = "DOES NOT MATCH";
		return false;
	}
	regfree(&re);

	return true;
}