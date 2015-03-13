#ifndef CRAWLER_H
#define CRAWLER_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "debug.h"


int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);
int getpage(char *url);
int process_page(char* text, int size, char *url);

#endif
