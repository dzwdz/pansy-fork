#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// not actually http
int main() {
	// create socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		puts("socket() err");
		return 1;
	}

	struct sockaddr_in my_addr = {0};
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(1312);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	// bind and listen
	if (bind(sockfd, (void*)&my_addr, sizeof my_addr) == -1) {
		puts("bind() err");
		return 1;
	}
	if (listen(sockfd, 10) == -1) {
		puts("listen() err");
		return 1;
	}

	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof client_addr;
	
	for (;;) {
		int client_fd = accept(sockfd, (void*)&client_addr, &addr_size);

		char *msg = "THIS WORKS\n";

		/* int bytes_sent = */ send(client_fd, msg, strlen(msg), 0);

		close(client_fd);
	}
}

