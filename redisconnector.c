/****************************************************************************** 
 * 
 * Description:	Redis connector for cRawler	
 *
 * Version:	1.0
 * Revision: 	1.0
 * Created:	2015-03-11 Wed 12:28 PM
 * Compiler: 	clang --std=c11
 *
 * Author:	michael.osiecki
 * Org: 	kohls
 *
 ******************************************************************************/
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

#define IP 127.0.0.1
#define PORT 6379
#define REDISTIMEOUT 500000
static redisContext *c;

struct keyvalue {
	char* key;
	char* value;
};

int connect_to_redis(char *ip, int port) {
	struct timeval timeout = {1, REDISTIMEOUT};
	c = redisConnectWithTimeout(ip, port, timeout);
	if (c != NULL && c->err) {
		log_err("Error making redis connectinon: %s", c->errstr);
	}
	log_info("Redis connection made!");
	redisReply *reply;
	reply = redisCommand(c, "PING");
	log_info("Ping-> %s", reply->str);
	freeReplyObject(reply);
	return 0;
}

int set_key(char *key, char *value) {
	redisReply *reply;
	reply = redisCommand(c, "SET %s %s", key, value);
	//log_info("SET: %s", reply->str);
	freeReplyObject(reply);
	return 0;
}
char *get_value(char *key) {
	char *ret;
	redisReply *reply;
	reply = redisCommand(c, "GET %s", key);
	ret = reply->str;
	return ret;
}

struct keyvalue *get_random_key() {
	struct keyvalue *ret = NULL;
	redisReply *reply;
	reply = redisCommand(c, "RANDOMKEY");
	printf("%s\n", reply->str);
	printf("%s\n", get_value(reply->str));
	freeReplyObject(reply);
	return ret;
}

