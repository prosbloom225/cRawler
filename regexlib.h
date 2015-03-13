#ifndef __REGEX_H__
#define __REGEX_H__
#include "debug.h"
#include <stdlib.h>
#include <regex.h>
#include "redisconnector.h"

int getproducts(size_t size, char *data);
int free_regex();

#endif
