#ifndef __REGEX_H__
#define __REGEX_H__
#include "debug.h"
#include <stdlib.h>
#include <regex.h>
#include "redisconnector.h"

int getproducts(size_t size, char *data);
int regeximages(size_t size, char *data, char *url);
int getetag(size_t size, char *data, char *fake_eTag);
int free_regex();

#endif
