/****************************************************************************** 
 * 
 * Description:	Crawler
 *
 * Version:	1.0
 * Revision: 	1.0
 * Created:	2015-03-09 Mon 02:28 PM
 * Compiler: 	clang --std=c11
 *
 * Author:	michael.osiecki
 * Org: 	none
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <curl/curl.h>
#include <pthread.h>
#include "debug.h"
#include "redisconnector.h"
#include "crawler.h"

#define DEBUG 1
#define HOST "www.kohls.com"
#define PORT 80
#define USERAGENT "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.115"
#define REDIS_SERVER "127.0.0.1"
#define REDIS_PORT 6379

struct keyvalue{
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

int getpage(char *url) {
	// Build URL
	char buf[256];
	snprintf(buf, sizeof buf, "%s%s", HOST, url);
	url = buf;
#ifdef DEBUG
	log_info("Curling for url: %s", url);
#endif

	CURL *curl_handle;
	CURLcode res;
	struct MemoryStruct chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "hurr durr im a sheep");
	// Exec!
	res =  curl_easy_perform(curl_handle);

	if (res != CURLE_OK) {
		log_err("curl_easy_perform failed: %s", curl_easy_strerror(res));
	} else {
		// We got data, process
#ifdef DEBUG
		log_info("Curl: %lu bytes retrieved", (long)chunk.size);
#endif
		process_page(chunk.memory, chunk.size, url);
	}
	curl_easy_cleanup(curl_handle);
	if (chunk.memory)
		free(chunk.memory);
	curl_global_cleanup();
	log_info("getpage complete");
	return 0;
}

int process_page(char* text, int size, char *url) {
#ifdef DEBUG
	log_info("Processing page: %s", url);
#endif 
	if (size != 0) {
		text = NULL;
	}
	return 0;
}

int main (int argc, char **argv) {
	log_info("Worker initialized");
	if (argc != 0) {
		getpage(argv[1]);
	}
	worker_loop();
	return 0;
}

void *worker_loop() {
	pthread_t id = pthread_self();
	log_info("Starting worker loop for thread: %lu", (long)id);
	connect_to_redis(NULL,0);
	while (1) {
		log_info("Working...");
		struct keyvalue k = pop_random_key();
		if (k.key != 0) {
		log_info("KEY: %s", k.key);
		log_info("VAL: %s", k.value);
		getpage(k.key);
		} else {
			log_info("NO KEY RETURNED");
		}
		// TODO - Remove this sleep, the processing of the page and http wait time should be enough sleep
		sleep(1);
	}
#ifdef DEBUG
	log_info("Worker complete.  Closing up shop.");
#endif
	close_redis();
	return NULL;
}
