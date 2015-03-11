#include "debug.h"
#include <stdlib.h>
#include <regex.h>
#include "redisconnector.h"


int getproducts(size_t size, char *data) {
	regex_t regex;
	int reti;
	char msg[256];

	if (size <=0) {
		log_err("Please pass a valid sized data");
		return -1;
	}

	reti = regcomp(&regex, "^0", REG_NEWLINE);
	if (reti) {
	fprintf(stderr, "Could not compile regex");
	exit(1);
	}
	reti = regexec(&regex, data, 0, NULL, 0);
	if (!reti) {
		log_info("MATCH");
	} else if (reti == REG_NOMATCH) {
		printf("NO MATCH\n");
	} else {
		regerror(reti, &regex, msg, sizeof(msg));
		log_err("Regex failed: %s\n", msg);
		exit(1);
	}
	return 0;
}
