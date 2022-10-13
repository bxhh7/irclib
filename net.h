/* if you need to use any other networking api, all you need to do is to implement
 * this interface */ 
#pragma once

#include <sys/types.h> // size_t 
typedef int irc_socket_t; /* network handle type, change this to what is
							appropriate for your api, we're using the berekeley sockets.*/

/* connect to server:port, and return a "handle" that will passed to irc_sock_recv() and irc_irc_sock_send() functioons*/
irc_socket_t irc_socket_create(char *server, char *port); 

/* reads at most max bytes from the sock and puts into the buffer,
 * returns the number of bytes read on success, and a negative value on failure */
ssize_t irc_socket_recv(irc_socket_t sock, char* buffer, size_t max);

/* writes size bytes from the buffer into irc_socket_t, 
 * returns the number of bytes that were written into sock on sucess, and a negative
 * value on failure.*/
ssize_t irc_socket_send(irc_socket_t sock, char* buffer, size_t size); 
