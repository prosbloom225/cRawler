/****************************************************************************** 
 * 
 * Description:	Master Crawler Control
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
#include <stdio.h>
#include <sys/types.h>
#include <curl/curl.h>
#include "crawler.h"
#include "debug.h"
#include "regexlib.h"

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

int main(int argc, char **argv) {
	if (argc == 0) {
		return 0;
	}
	pid_t pid;


	CURL *curl_handle;
	CURLcode res;
	struct MemoryStruct chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "firefox");
	// Exec!
	res =  curl_easy_perform(curl_handle);

	if (res != CURLE_OK) {
		log_err("curl_easy_perform failed: %s", curl_easy_strerror(res));
	} else {
		// We got data, process
		log_info("Curl: %lu bytes retrieved", (long)chunk.size);
		//log_info("%s\n", chunk.memory);
		// Get the products
		getproducts(chunk.size, chunk.memory);
		
	}
	curl_easy_cleanup(curl_handle);
	if (chunk.memory)
		free(chunk.memory);
	curl_global_cleanup();

	log_info("Master complete");
	return 0;
}
