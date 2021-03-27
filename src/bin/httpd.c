#include "macros.h"
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char *BAD_REQ = "\
HTTP/1.1 400 Bad Request\r\n\
Content-Type: text/html\r\n\
Connection: close\r\n\
\r\n\
<h1>400 Bad Request</h1>\
<hr><center>httpd (pansy Linux)</center>";

const char *GOOD_REQ = "\
HTTP/1.1 200 OK\r\n\
\r\n";

void handle_conn(int fd) {
	bool answered = false;

	char *buf = malloc(1024);
	int bytes = recv(fd, buf, 1024, 0);
	if (bytes <= 0) {
		free(buf);
		return;
	}
	char *end = buf + bytes;
	write(1, buf, bytes);

	// parse the path
	while (buf < end) {
		if (*buf++ == ' ') break;
	}
	if (buf < end) {
		char *path = buf;
		for (; buf < end; buf++) {
			if (*buf == ' ') break;
		}
		if (buf < end) {
			*buf = '\0';

			// todo: buffer overflow
			// also, if the path doesn't start with / it accesses other directories
			// which i guess is a vuln?
			// also, /../ and /~/
			char *path2 = malloc(1024);
			strcpy(path2, "/var/www/html");
			strcpy(path2 + 13, path);

			int fd2 = open(path2, 0);
			if (fd2) {
				struct stat stat;
				if (!fstat(fd2, &stat)) {
					void *contents = malloc(stat.st_size);
					// i'm tired of error handling let's just assume that
					// i got the memory
					read(fd2, contents, stat.st_size);
					close(fd2);

					send(fd, GOOD_REQ, strlen(GOOD_REQ), 0);

					// this might not send the whole file
					send(fd, contents, stat.st_size, 0);
					answered = true;

					free(contents);
				}
				close(fd2);
			}

			free(path2);
		}
	}

	if (!answered)
		send(fd, BAD_REQ, strlen(BAD_REQ), 0);
	free(buf);
}

int main() {
	// create socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) DIE("socket() err", 1);

	struct sockaddr_in my_addr = {0};
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(80);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	// bind and listen
	if (bind(sockfd, (void*)&my_addr, sizeof my_addr) == -1)
		DIE("bind() err", 1);
	if (listen(sockfd, 10) == -1) DIE("listen() err", 1);

	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof client_addr;
	
	for (;;) {
		int client_fd = accept(sockfd, (void*)&client_addr, &addr_size);

		handle_conn(client_fd);

		close(client_fd);
	}
}

