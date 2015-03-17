/****************************************************************************** 
 * 
 * Description:	Master Crawler Control
 * 		Controls spinning up the slavedriver and handles crawling of
 * 		catalog.jsp pages
 *
 * Version:	1.0
 * Revision: 	1.0
 * Created:	2015-03-10 Tue 02:58 PM
 * Compiler: 	clang --std=c11
 *
 * Author:	michael.osiecki
 * Org: 	kohls
 *
 ******************************************************************************/
// clang doesnt like nanosleep for some reason....
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <unistd.h>
#include <time.h>
#include "crawler.h"
#include "debug.h"
#include "regexlib.h"
#include "slavedriver.h"


#define DEBUG 1
#define URL "www.kohls.com/catalog.jsp?N=0&WS="
#define MAXPRDPAGES 200000


static int min_page=0;
static int max_page;
struct keyvalue {
	char* key;
	char* value;
};
struct MemoryStruct {
	char *memory;
	size_t size;
};
static size_t WriteMemoryCallback (void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size *nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *) userp;
	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory ==NULL) {
		// out of memory
		log_err("Out of memory!");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

void process_args(int argc, char **argv) {
#ifdef DEBUG
	log_info("Setting start/end page");
	log_info("Start: %d", atoi(argv[1]));
	log_info("End  : %d", atoi(argv[2]));
#endif
	if (argc == 3) {
	min_page = atoi(argv[1]);
	max_page = atoi(argv[2]);
	}
	log_info("%d", max_page);
}

int main(int argc, char **argv) {
	struct timespec sleep_time;
	sleep_time.tv_sec=0;
	sleep_time.tv_nsec = 500000000L;
	max_page = MAXPRDPAGES;
	
	char *url = URL;
	if (argc > 1) {
		process_args(argc, argv);
	} else {
	}

	// init redis connection
	int rc;
	rc = connect_to_redis(NULL,0);
	log_info("Redis connection OK!");

#ifdef DEBUG
	log_info("Building catalog page database");
#endif
	char *data = malloc(sizeof(*data) * 255 ); 
	char **catalogDB = malloc(sizeof(char *) * max_page);
	for (int i = 0;i < max_page; ++i) {
		catalogDB[i] = malloc(sizeof(char *) * 255);
	}

	long i;
	// build database of catalog0 pages
	for (i=0;i < (max_page/96); i++){
		long s = i*96 + (min_page - min_page%96);
		char  c[40];
		sprintf(c, "%lu", s);

		char currPage[80];
		snprintf(currPage, sizeof currPage, "%s%s" , URL, c);
		memcpy(catalogDB[i], currPage, sizeof(currPage));
	}
#ifdef DEBUG
		log_info("CatalogDB built! %lu pages queued",  i);
		log_info("Last page: %s", catalogDB[i-1]);
#endif


	// // BEGIN THE MAIN LOOP

	for (int i=0;i < (max_page/96); i++){
#ifdef DEBUG
		log_info("Beginning curl for: %s", catalogDB[i]);
#endif

		// curl the page
		CURL *curl_handle;
		CURLcode res;
		struct MemoryStruct chunk;
		chunk.memory = malloc(1);
		chunk.size = 0;
		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_URL, catalogDB[i]);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "hurr durr im a sheep");
		// Exec!
		res =  curl_easy_perform(curl_handle);

		if (res != CURLE_OK) {
			log_err("curl_easy_perform failed: %s", curl_easy_strerror(res));
			log_err("URL: %s", catalogDB[i]);
			log_err("DEFAULT: %s", catalogDB[i]);
		} else {
			// We got data, process
#ifdef DEBUG
			log_info("Curl: %lu bytes retrieved", (long)chunk.size);
			//log_info("MEM: %s", chunk.memory);
#endif
			// Get the products
			getproducts(chunk.size, chunk.memory);

		}
		curl_easy_cleanup(curl_handle);
		if (chunk.memory)
			free(chunk.memory);
		curl_global_cleanup();

		int s = get_dbsize();
		log_info("DB Size: %i", s);
		
		// Wait until we have less data, then cycle back
		while (s >= 192){
			//sleep(1);
			nanosleep(&sleep_time, NULL);
			s = get_dbsize();
#ifdef DEBUG
		log_info("DB Size: %i", s);
#endif
		} 
	}


	// Clean up
	free_regex();
	close_redis();
	log_info("Master complete");
	return 0;
}

