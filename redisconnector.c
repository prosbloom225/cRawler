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

#define DEBUG 1
#define IP 127.0.0.1
#define PORT 6379
#define REDISTIMEOUT 500000
static redisContext *c;

struct keyvalue{
	char* key;
	char* value;
};

int connect_to_redis(char *ip, int port) {
	//struct timeval timeout = {1, REDISTIMEOUT};
	c = redisConnect(ip, port);
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
void close_redis() {
	redisFree(c);
}

int print_reply(redisReply *reply) {
	log_warn("STR: %s", reply->str);
	log_warn("INT: %lu", (long)reply->integer);
	log_warn("LEN: %i", reply->len);
	return 0;
}
int set_key(char *key, char *value) {
	redisReply *reply;
	reply = redisCommand(c, "SET %s %s", key, value);
#ifdef DEBUG
	log_info("SET: %s, %s", key, value);
	print_reply(reply);
#endif
	freeReplyObject(reply);
	return 0;
}
char *get_value(char *key) {
	log_info("Getting value for key: %s", key);
	redisReply *reply;

	reply = redisCommand(c, "GET %s", key);
#ifdef DEBUG
	log_info("Got val: %s", reply->str);
	print_reply(reply);
#endif

	// check for bad malloc's
	if (reply->len == 0) {
		return "NO MATCH";
	}
	// alloc the reply value
	char *ret;
	ret = malloc(reply->len);
	memcpy(ret, reply->str, reply->len);
#ifdef DEBUG
	log_err("RET: %s", ret);
	log_err("SIZE: %lu", (long)reply->len);
	log_info("Freeing get_value reply");
#endif
	freeReplyObject(reply);
	return ret;
}
int del_key(char *key) {
	redisReply *reply;
	reply = redisCommand(c, "DEL %s ", key);
	log_info("Deleted key: %s", key);
	freeReplyObject(reply);
	return reply->integer;
}

struct keyvalue get_random_key() {
	log_info("Returning random key");
	redisReply *reply;
	reply = redisCommand(c, "RANDOMKEY");
	log_info("Got random key");
#ifdef DEBUG
	print_reply(reply);
#endif

	// alloc the key
	char key[reply->len];
	//*key = calloc(0,reply->len);
	memcpy(key, reply->str, reply->len+1);
	log_info("KEY: %s\n", key);
	log_info("KEY: %s\n", reply->str);

	// get the val
	char *val = get_value(key);
#ifdef DEBUG
	log_info("KEY: %s", key);
	log_info("VAL: %s", val);
#endif

	// build return obj
	struct keyvalue ret;
	ret.key = key;
	ret.value = val;
#ifdef DEBUG
	log_info("Freeing get_random_key reply");
#endif
	freeReplyObject(reply);
	return ret;
}

struct keyvalue pop_random_key() { 
	struct keyvalue ret = get_random_key();
	log_info("POPKEY %s", ret.key);
	log_info("POPVAL %s", ret.value);
	//del_key(ret.key);
	log_info("Returning obj");
	return ret;
}

int get_dbsize() {
	redisReply *reply;
	reply = redisCommand(c, "DBSIZE");
	int ret;
	ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

