#ifndef __REDISCONNECTOR_H
#define __REDISCONNECTOR_H
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

int connect_to_redis(char *ip, int port);
int set_key(char *key, char *value);
struct keyvalue *get_random_key();

#endif
