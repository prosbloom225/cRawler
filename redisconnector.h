#ifndef __REDISCONNECTOR_H
#define __REDISCONNECTOR_H
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

int connect_to_redis(char *ip, int port);
int connect_to_redis2(char *ip, int port);
void close_redis();
int set_key(char *key, char *value);
int set_key2(char *key, char *value);
struct keyvalue *get_random_key();
struct keyvalue pop_random_key(); 
int del_key(char *key);
int get_dbsize();

int print_reply(redisReply *reply);
int flushall();
int flushall2();
int check_exists(char *key);
int check_exists2(char *key);
#endif
