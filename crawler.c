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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);
void usage();

#define HOST "www.kohls.com"
#define PAGE "/"
#define PORT 80
#define USERAGENT "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.115"


char* getpage(char *host, char *page) {
	struct sockaddr_in *remote;
	int sock;
	int tmpres;
	char *ip;
	char *get;
	char buf[BUFSIZ+1];

	sock = create_tcp_socket();
	ip = get_ip(host);
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
	tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
	if (tmpres < 0) {
		log_err("Cant set remote->sin_addr.s_addr");
		exit(1);
	} else if (tmpres == 0) {
		log_err("%s is not a valid IP address\n", ip);
		exit(1);
	}
	remote->sin_port = htons(PORT);

	if (connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0) {
		log_err("Could not connect");
		exit(1);
	}

	get = build_get_query(host,page);
	log_info("Query is: \n<<START>>\n%s\n<<END>>\n", get);

	// Send the query
	int sent = 0;
	while (sent < (int)strlen(get)) {
		tmpres = send(sock, get+sent, strlen(get)-sent, 0);
		if(tmpres == -1) {
			log_err("Cant send query");
			exit(1);
		}
		sent += tmpres;
	}
	// Receive the page
	memset(buf, 0, sizeof(buf));
	int htmlstart =0;
	char *htmlcontent;
	while ((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0) {
		if (htmlstart ==0) {
			// strip the header
			htmlcontent = strstr(buf, "<!DOCTYPE");
			if (htmlcontent != NULL) {
				htmlstart=1;
				htmlcontent += 4;
			} 
		}else {
			htmlcontent = buf;
		}
		if (htmlstart) {
			fprintf(stdout, htmlcontent);
			// concat string
			//char buffer[sizeof(buf)+sizeof(htmlcontent)];
			//snprintf(buffer, sizeof buffer, "%s%s", htmlcontent,buf);
		}
		memset(buf, 0, tmpres);
	}
	if (tmpres < 0) {
		log_err("Error receiving data");
	}
	free(get);
	free(remote);
	free(ip);
	close(sock);
	return 0;

	// Default return
	return NULL;
}

int create_tcp_socket() {
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		log_err("Cant create TCP socket");
		exit(1);
	}
	return sock;
}

char *get_ip(char *host) {
	struct hostent *hent;
	int iplen= 15;
	char *ip = (char *)malloc(iplen+1);
	memset(ip,0,iplen+1);
	if ((hent = gethostbyname(host)) == NULL) {
		log_err("Cant get IP");
		exit(1);
	}
	if (inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL) {
		log_err("Cant resolve host");
		exit(1);
	}
	return ip;
}

char *build_get_query(char *host, char *page) {
	char *query;
	char *getpage = page;
	char *tpl = "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nConnection: keep-alive\r\n\r\n";
	if (getpage[0] == '/') {
		getpage = getpage +1;
		log_warn("Removing leading \"/\", converting %s to %s\n", page, getpage);
	}
	query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
	sprintf(query, tpl, getpage, host, USERAGENT);
	return query;
}
