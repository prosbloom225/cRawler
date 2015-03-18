/****************************************************************************** 
 * 
 * Description:	Regex lib for cRawler
 *
 * Version:	1.0
 * Revision: 	1.0
 * Created:	2015-03-11 Wed 16:27 PM
 * Compiler: 	clang --std=c11
 *
 * Author:	michael.osiecki
 * Org: 	kohls
 *
 ******************************************************************************/
#include "debug.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "redisconnector.h"

//#define DEBUG 1
static regex_t catalog_regex;
static regex_t image_regex;

struct keyvalue {
	char* key;
	char* value;
};

static int compile_regex(regex_t *r, const char *regex_text) {
	int status = regcomp(r, regex_text, REG_EXTENDED|REG_NEWLINE);
	if (status != 0) {
		char err_msg[256];
		regerror(status, r, err_msg, 256);
		log_err("Regex error compiling '%s': %s", regex_text, err_msg);
		return 1;
	}
#ifdef DEBUG
	log_info("Regex compiled!");
#endif


	return 0;
}
void free_regex() {
	regfree(&catalog_regex);
}

int getproducts(size_t size, char *data) {
	//regex_t regex;
	int reti;
	char msg[256];

	if (size <=0) {
		log_err("Please pass a valid sized data");
		return -1;
	}
#ifdef DEBUG
	log_info("Size of data: %lu", (long)size);
#endif

	char *regex_text= "/product/prd-.*\\.jsp";
	if (catalog_regex.__allocated == 0) {
	log_info("Compiling regex: %s", regex_text);
	compile_regex(&catalog_regex, regex_text);
	}

	const char *p = data;
	const int n_matches = 256;
	regmatch_t m[n_matches];
	int count = 0;
	while (1) {
		int i=0;
		int nomatch = regexec(&catalog_regex, p, n_matches, m, 0);
		if (nomatch) {
#ifdef DEBUG
			log_info("Regex complete with %d matches", count);
#endif
			return nomatch;
		}
		for (i=0;i < n_matches;i++) {
			count++;
			int start;
			int finish;
			if (m[i].rm_so == -1) {
				break;
			}
			start = m[i].rm_so + (p-data);
			finish = m[i].rm_eo + (p-data);
			if (i ==0) {
				//printf("$& is");
			} else {
				//printf("$%d is", i);
			}
			//printf("'%.*s' (bytes %d:%d)\n", (finish-start), data+start, start, finish);
#ifdef DEBUG
			/* log_info("START: %i", start); */
			/* log_info("FINSH: %i", finish); */
			/* log_info("SFIN : %c", data[finish]); */
#endif
			char substring[finish-start];
			memcpy(substring, &data[start], (finish-start));
			// Terminate the char* as a string
			substring[finish-start] = '\0';
			set_key(substring, "catalog.jsp");
#ifdef DEBUG
			log_info("PRD: %.*s", (finish-start),data+start);
			log_info("SENT: %s", substring);
#endif
		}
		p += m[0].rm_eo;
	}
	return 0;
}


int getimages(size_t size, char *data, char *url) {
#ifdef DEBUG
	log_info("getimages begin");
#endif
	//regex_t regex;
	int reti;
	char msg[256];

	if (size <=0) {
		log_err("Please pass a valid sized data");
		return -1;
	}
#ifdef DEBUG
	log_info("Size of data: %lu", (long)size);
#endif

	//char *regex_text= "/product/prd-.*\\.jsp";
	char *regex_text= "http://media[0-9]?\\.kohls\\.com\\.edgesuite\\.net(.*)&op_sharpen=1";
	if (image_regex.__allocated == 0) {
#ifdef DEBUG
	log_info("Compiling regex: %s", regex_text);
#endif
	compile_regex(&image_regex, regex_text);
	}

	const char *p = data;
	const int n_matches = 256;
	regmatch_t m[n_matches];
	int count = 0;
	while (1) {
		int i=0;
		int nomatch = regexec(&image_regex, p, n_matches, m, 0);
		if (nomatch) {
#ifdef DEBUG
			log_info("Regex complete with %d matches", count);
#endif
			return nomatch;
		}
		for (i=0;i < n_matches;i++) {
			count++;
			int start;
			int finish;
			if (m[i].rm_so == -1) {
				break;
			}
			start = m[i].rm_so + (p-data);
			finish = m[i].rm_eo + (p-data);
			if (i ==0) {
				//printf("$& is");
			} else {
				//printf("$%d is", i);
			}
			//printf("'%.*s' (bytes %d:%d)\n", (finish-start), data+start, start, finish);
#ifdef DEBUG
			/* log_info("START: %i", start); */
			/* log_info("FINSH: %i", finish); */
			/* log_info("SFIN : %c", data[finish]); */
#endif
			char substring[finish-start];
			memcpy(substring, &data[start], (finish-start));
			// Terminate the char* as a string
			substring[finish-start] = '\0';
			// Push the data to pagesToVisit
			//set_key2(substring, url);
#ifdef DEBUG
			log_info("PRD: %.*s", (finish-start),data+start);
			log_info("SENT: %s", substring);
			log_info("URL: %s", url);
#endif
		}
		p += m[0].rm_eo;
	}
	return 0;
}
