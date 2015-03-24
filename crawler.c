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
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <pthread.h>
#include "regexlib.h"
#include "debug.h"
#include "redisconnector.h"
#include "crawler.h"

//#define DEBUG 1
#define HOST "http://www.kohls.com"
//#define USERAGENT "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.115"
#define USERAGENT "hurrdurrimasheep"
#define FAKEETAGADDRESS "http://media.kohls.com.edgesuite.net/is/image/kohls/1234fakeimage"
//?wid=&hei=&op_sharpen=1"
static char *redis_server_ip; 
static int redis_server_port;
static char **fake_etags;

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
	// Build URL if needed
	if (strncmp(url, "http://", 7) != 0) {
		char buf[256];
		snprintf(buf, sizeof buf, "%s%s", HOST, url);
		url = buf;
	}
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

	if (size == 0) {
		text = NULL;
	} else {
		getimages(url);
	}
	return 0;
}

int check_etag(char *etag) {
	for (int i=0; i < 7;i++) {
		if (strcmp(etag, fake_etags[i]) == 0) {
			// Image is coming soon
			return 1;
		} 
	}
	// Image is real
	return 0;
}

char *get_fake_etag(int x, int y) {
#ifdef DEBUG
		log_info("Getting fake etag");
#endif
		// build etag address ...ugh
		// x
		char xbuf[20];
		sprintf(xbuf, "%d", x);
		size_t xsiz = strlen(xbuf);
		log_info("%lu", xsiz);
		char xwid[6 + xsiz];// = "?wid=";
		snprintf(xwid, sizeof(xwid), "%s%s", "?wid=", xbuf);
		log_info("WID: %s", xwid);
		// y
		char ybuf[20];
		sprintf(ybuf, "%d", y);
		size_t ysiz = strlen(ybuf);
		log_info("%lu", ysiz);
		char yhei[6 + ysiz];// = "&hei=";
		snprintf(yhei, sizeof(yhei), "%s%s", "&hei=", ybuf);
		log_info("HEI: %s", yhei);
		// now kiss
		char buf[65 + 6+xsiz + 6+ysiz];
		snprintf(buf, sizeof buf, "%s%s%s",FAKEETAGADDRESS, xwid, yhei);

#ifdef DEBUG
		log_info("Curling for URL: %s", buf);
#endif
		CURL *curl_handle;
		CURLcode res;
		struct MemoryStruct chunk;
		chunk.memory = malloc(1);
		chunk.size = 0;
		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_URL, buf);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		// Get headers only
		curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);

		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USERAGENT);
		// Exec!
		res =  curl_easy_perform(curl_handle);

		if (res != CURLE_OK) {
			log_err("curl_easy_perform failed: %s", curl_easy_strerror(res));
		} else {
			// We got data, process
#ifdef DEBUG
			log_info("Curl: %lu bytes retrieved", (long)chunk.size);
#endif
		}
		curl_easy_cleanup(curl_handle);
#ifdef DEBUG
		log_info("%s", chunk.memory);
#endif
		if (chunk.memory)
			free(chunk.memory);
		curl_global_cleanup();
#ifdef DEBUG
		char *ret = getetag(chunk.size, chunk.memory);
		log_info("etag received %s", ret);
		return ret;
#endif
	}

void build_fake_etags() {
	fake_etags = malloc(33 * 7);
	fake_etags[0] = get_fake_etag(20,20);
	fake_etags[1] = get_fake_etag(30,30);
	fake_etags[2] = get_fake_etag(50,50);
	fake_etags[3] = get_fake_etag(75,75);
	fake_etags[4] = get_fake_etag(350,350);
	fake_etags[5] = get_fake_etag(400,400);
	fake_etags[6] = get_fake_etag(1000,1000);
#ifdef DEBUG
	log_info("Fake etags db built!");
	for (int i=0; i < 7; i++) {
		log_info("%s", fake_etags[i]);
	}
#endif
}

	void getimage (char *url) {
		// This is an akamaized image, just snag the header
#ifdef DEBUG
		log_info("IMG: %s", url);
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
		// Get headers only
		curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);

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
			char *etag = getetag(chunk.size, chunk.memory);
			int rc =  check_etag(etag);;
			if (rc == 1) {
				log_info("Image coming soon!");
				log_info("ETAG: %s", etag);
				log_info("URL: %s", url);
				exit(EXIT_SUCCESS);
			} else {
				log_info("Valid image!");
			}
			free(etag);

		}
		curl_easy_cleanup(curl_handle);
#ifdef DEBUG
		log_info("%s", chunk.memory);
#endif
		if (chunk.memory)
			free(chunk.memory);
		curl_global_cleanup();
#ifdef DEBUG
		log_info("getimage complete");
#endif

		//exit(EXIT_SUCCESS);
	}

	void print_usage() {
		printf("cRawler worker usage: \n");
		printf("worker redis_server_ip redis_server_port");
		exit(EXIT_FAILURE);
	}

	void process_args(int argc, char **argv) {
		// ./worker redis_server redis_port
		if (argc == 3) {
#ifdef DEBUG
			log_info("Setting redis_server based on args");
			for (int i = 0; i < argc; i++) 
				log_info("%d: %s", argc, argv[i]);
#endif
			redis_server_ip = argv[1];
			redis_server_port = atoi(argv[2]);
		} else if (argc == 1) {
			redis_server_ip = "127.0.0.1";
			redis_server_port = 6379;
		} else {
			print_usage();
		}
	}

	int main (int argc, char **argv) {
		build_fake_etags();
		//testing
		//exit(EXIT_SUCCESS);

#ifdef DEBUG
		log_info("Worker initialized");
#endif
		process_args(argc, argv);
		worker_loop();
		return 0;

	}

	uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out) {
		uint r;
		r = size * nmemb;
		tidyBufAppend(out, in, r);
		return r;
	}

	void dumpNode(TidyDoc doc, TidyNode tnod, int indent, char *url) {
		TidyNode child;
		for (child = tidyGetChild(tnod);child;child = tidyGetNext(child)){
			ctmbstr name = tidyNodeGetName(child);
			if (name) {
				// if it has a name its an html tag	
				TidyAttr attr;
				// ignore tags
				//log_info("TAG:  %*.*s%s", indent, indent, "<", name);
				// walk the attribute list 
				for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
					if (strncmp(tidyAttrName(attr), "src", 3) == 0) { 
						if(strncmp(tidyAttrValue(attr), "http://media", 12) == 0) {
							// We have an image, push
							tidyAttrValue(attr)?log_info("ATTR: %s =\"%s\" ",tidyAttrName(attr),
									tidyAttrValue(attr)):log_err(" ");
							set_key((char *)tidyAttrValue(attr), url);
						}
					}
				}
			} 
			dumpNode(doc, child, indent + 4, url); // recursive
		}
	}

	void *getimages(char *url) {
		CURL *curl;
		char curl_errbuf[CURL_ERROR_SIZE];
		TidyDoc tdoc;
		TidyBuffer docbuf = {0, NULL, 0, 0, 0};
		TidyBuffer tidy_errbuf = {0, NULL, 0, 0, 0};
		int err;
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

		tdoc = tidyCreate();
		//tidyOptSetBool(tdoc, TidyWrapLen, 9999);
		tidySetErrorBuffer(tdoc, &tidy_errbuf);
		tidyBufInit(&docbuf);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
		err = curl_easy_perform(curl);
		if (!err) {
			err = tidyParseBuffer(tdoc, &docbuf);
			if (err >= 0) {
				err = tidyCleanAndRepair(tdoc);
				if (err >= 0) {
					//err = tidyRunDiagnostics(tdoc);
					if (err >= 0) {
						dumpNode(tdoc, tidyGetRoot(tdoc), 0, url);
						//log_err("%s", tidy_errbuf.bp);
					}
				}
			}
		}
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		tidyBufFree(&docbuf);
		tidyBufFree(&tidy_errbuf);
		tidyRelease(tdoc);
		return NULL;
	}

	void *worker_loop() {
		pthread_t id = pthread_self();
		log_info("Starting worker loop for thread: %lu", (long)id);
		connect_to_redis(redis_server_ip,redis_server_port);
		connect_to_redis2(redis_server_ip,redis_server_port+1);
		int cycles = 0;
		while (1) {
			log_info("Working...");
			struct keyvalue k = pop_random_key();
			if (k.key != 0) {
				log_info("KEY: %s", k.key);
				log_info("VAL: %s", k.value);
				if (1) {
					/* Process the page
					 *  if it's an image break out and process_image
					 *  else process_page
					 */
					if (strncmp(k.key, "http://media", 12) == 0) {
						// image
						getimage(k.key);
					} else {
						// page
						getpage(k.key);
					}
					// We're done with the page, push to pagesVisited;
					set_key2(k.key, k.value);
					free(k.key);
					free(k.value);
				} else {
					// Process images
				}
			} else {
				log_info("NO KEY RETURNED");
			}

			// TODO - Remove this sleep, the processing of the page and http wait time should be enough sleep
			sleep(1);
			// testing
			if (cycles++ >=50)
				break;
		}
#ifdef DEBUG
		log_info("Worker complete.  Closing up shop.");
#endif
		close_redis();
		return NULL;
	}
