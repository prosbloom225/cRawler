/****************************************************************************** 
 * 
 * Description:	Redis connector for cRawler	
 * 		Provides an api for interacting with redis.  Returns key_value
 * 		pairs.
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

//#define DEBUG 1
#define IP "127.0.0.1"
#define PORT 6379
#define REDISTIMEOUT 500000
// PagesToVisit
static redisContext *c;
// PagesVisited
static redisContext *d;

struct keyvalue{
	char* key;
	char* value;
};

int connect_to_redis(char *ip, int port) {
#ifdef DEBUG
	log_info("Opening redis connction to server: %s:%d", ip, port);
#endif
	if (ip == NULL)
		ip = IP;
	if (port == 0) 
		port = PORT;
	//struct timeval timeout = {1, REDISTIMEOUT};
	c = redisConnect(ip, port);
	if (c != NULL && c->err) {
		log_err("Error making redis connectinon: %s", c->errstr);
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG
	log_info("Redis connection made!");
#endif 
	redisReply *reply;
	reply = redisCommand(c, "PING");
#ifdef DEBUG
	log_info("Ping-> %s", reply->str);
#endif
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
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}
char *get_value(char *key) {
#ifdef DEBUG
	log_info("Getting value for key: %s", key);
#endif
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
	ret = malloc(reply->len+1);
	memcpy(ret, reply->str, reply->len+1);
#ifdef DEBUG
	log_err("RET: %s", ret);
	log_err("SIZE: %lu", (long)reply->len);
	log_info("Freeing get_value reply");
#endif
	freeReplyObject(reply);
	return ret;
}
int del_key(char *key) {
	if (key == NULL) {
		return -1;
	}
	redisReply *reply;
	reply = redisCommand(c, "DEL %s ", key);
#ifdef DEBUG
	log_info("Deleted key: %s", key);
#endif
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

struct keyvalue get_random_key() {
	struct keyvalue ret;
	ret.key = 0;
	ret.value = 0;
#ifdef DEBUG
	log_info("Returning random key");
#endif
	redisReply *reply;
	reply = redisCommand(c, "RANDOMKEY");
#ifdef DEBUG
	log_info("Got random key");
	print_reply(reply);
#endif

	if (reply->len == 0) {
		freeReplyObject(reply);
		return ret;
	}
	// alloc the key
	char *key;
	key = malloc(reply->len+1);
	memcpy(key, reply->str, reply->len+1);

	// get the val
	char *val = get_value(key);
#ifdef DEBUG
	log_info("After get_value");
	log_info("KEY: %s", key);
	log_info("VAL: %s", val);
#endif

	// build return obj
	ret.key = key;
	ret.value = val;
#ifdef DEBUG
	log_info("Freeing get_random_key reply");
#endif
	freeReplyObject(reply);
#ifdef DEBUG
	log_info("Returning struct: ");
	log_info("KEY: %s", ret.key);
	log_info("VAL: %s", ret.value);
#endif
	return ret;
}

struct keyvalue pop_random_key() { 
	struct keyvalue ret;
	ret = get_random_key();
#ifdef DEBUG
	log_info("POPKEY %s", ret.key);
	log_info("POPVAL %s", ret.value);
#endif
	if (ret.key != 0) {
		del_key(ret.key);
	}  else {
		log_info("Returning NULL keyvale");
		ret.key = 0;
		ret.value = 0;
	}
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


int connect_to_redis2(char *ip, int port) {
#ifdef DEBUG
	log_info("Opening redis connction to server: %s:%d", ip, port);
#endif
	if (ip == NULL)
		ip = IP;
	if (port == 0) 
		port = PORT;
	//struct timeval timeout = {1, REDISTIMEOUT};
	d = redisConnect(ip, port);
	if (d != NULL && d->err) {
		log_err("Error making redis connectinon: %s", c->errstr);
		exit(EXIT_FAILURE);
	} else {
		return 0;
	}
	return -1;
}
int set_key2(char *key, char *value) {
	redisReply *reply;
	reply = redisCommand(d, "SET %s %s", key, value);
#ifdef DEBUG
	log_info("SET: %s, %s", key, value);
	print_reply(reply);
#endif
	int ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

int flushall() {
	redisReply *reply;
	reply = redisCommand(c, "FLUSHALL");
#ifdef DEBUG
	log_info("FLUSHING REDIS");
	print_reply(reply);
#endif
	freeReplyObject(reply);
	return 0;
}

int flushall2() {
	redisReply *reply;
	reply = redisCommand(d, "FLUSHALL");
#ifdef DEBUG
	log_info("FLUSHING REDIS");
	print_reply(reply);
#endif
	freeReplyObject(reply);
	return 0;
}

int check_exists(char *key) {
	int ret = 0;
	redisReply *reply;
	reply = redisCommand(c, "EXISTS %s", key);
#ifdef DEBUG
	log_info("CHECKING EXISTS %s", key);
	print_reply(reply);
#endif
	ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}

int check_exists2(char *key) {
	int ret = 0;
	redisReply *reply;
	reply = redisCommand(d, "EXISTS %s", key);
#ifdef DEBUG
	log_info("CHECKING EXISTS %s", key);
	print_reply(reply);
#endif
	ret = reply->integer;
	freeReplyObject(reply);
	return ret;
}
