#include "net.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

irc_socket_t irc_socket_create(char* server, char* port) {
	struct addrinfo hints;
	struct addrinfo* result, *rp;
	int sockfd, s;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(server, port, &hints, &result);
	if (s != 0)
		return -1;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;
		s = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
		if (s != 0) {
			close(sockfd);
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	if (rp == NULL) 
		return -1;
	return sockfd;
}

ssize_t irc_socket_recv(irc_socket_t sock, char* buffer, size_t max) {
	return recv(sock, buffer, max, 0);
}

ssize_t irc_socket_send(irc_socket_t sock, char* buffer, size_t size) {
	return write(sock, buffer, size);
}
