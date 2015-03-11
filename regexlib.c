#include "debug.h"
#include <stdlib.h>
#include <regex.h>

static int compile_regex(regex_t *r, const char *regex_text) {
	int status = regcomp(r, regex_text, REG_EXTENDED|REG_NEWLINE);
	if (status != 0) {
		char err_msg[256];
		regerror(status, r, err_msg, 256);
		log_err("Regex error compiling '%s': %s", regex_text, err_msg);
		return 1;
	}
	log_info("Regex compiled!");
	return 0;
}

int getproducts(size_t size, char *data) {
	regex_t regex;
	int reti;
	char msg[256];

	if (size <=0) {
		log_err("Please pass a valid sized data");
		return -1;
	}
	log_info("Size of data: %lu", (long)size);

	char *regex_text= "/product/prd-.*\\.jsp";
	log_info("Compiling regex: %s", regex_text);
	compile_regex(&regex, regex_text);

	const char *p = data;
	const int n_matches = 256;
	regmatch_t m[n_matches];
	while (1) {
		int i=0;
		int nomatch = regexec(&regex, p, n_matches, m, 0);
		if (nomatch) {
			log_info("No more matches");
			return nomatch;
		}
		for (i=0;i < n_matches;i++) {
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
			log_info("PRD: %.*s", (finish-start),data+start);
		}
		p += m[0].rm_eo;
	}
	return 0;
}
