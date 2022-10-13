#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <memory.h>
#include <stdarg.h>
#include <utils.h>
#include <stdint.h>


#define IRC_MAX_PARAMS 15 + 1 // one for the trailing
#define IRC_MAX_BUFFER 1024 
#define WORDL(c0, c1, c2, c3) (uint32_t)(c0 | (c1 << 8) | (c2 << 16) | (c3 << 24)) // shoutout to Hexchat 
#define IRC_MAX_MSG 512

typedef struct {
	char *prefix_name; // <servername> | <nick>
	char *prefix_user; // ['!' <user>]
	char *prefix_host; // ['@' <host>]
	char *cmd; // <cmd>
	char *parameters[IRC_MAX_PARAMS]; //<params>
	int parameters_size; // size of this guy ^
} irc_msg_t;

typedef struct irc_session irc_session_t;
typedef int (*event_handler_t)(struct irc_session* , irc_msg_t*); /* event handlers must return 0 on success, and non-zero values on failure */

typedef struct {
	event_handler_t privmsg_handler;
	event_handler_t join_handler;
	event_handler_t nick_handler;
	event_handler_t ping_handler;
	event_handler_t invite_handler;
} irc_event_handler_set_t;

typedef struct irc_session{
	char *server;
	char *portno;
	char *nick;
	char *password;
	
	char *buffer;
	int pos;

	int sockfd;
	int connected;
	irc_event_handler_set_t event_handlers;
} irc_session_t;


/* instances of irc_msg_t should ideally only be used through these macros. */
/* PRIVMSG handler helper macros */
#define IRC_PRIVMSG_IS_ORIGIN_CHANNEL(m)  m->parameters[0][0] == '#'
#define IRC_PRIVMSG_ORIGIN(m) IRC_PRIVMSG_IS_ORIGIN_CHANNEL(m) ? m->parameters[0] : m->prefix_name
#define IRC_PRIVMSG_MSG(m) m->parameters[1]

/* INVITE handler helper macros */
#define IRC_INVITE_CHANNEL(m) m->parameters[1]

/* PING handler helper macros */
#define IRC_PING_PARAMETER(m) m->parameters[0]

/* more can(and should) be added as the need for handling other events arises. */

/* functions */
int irc_init_session(irc_session_t* s, const char* serv, const char* portno, const char* nick, const char* pass, irc_event_handler_set_t* es);
int irc_create_socket(char *host, char* portno);
ssize_t irc_send_raw(irc_session_t* s, const char* format, ...);
int irc_connect(irc_session_t* s);
int irc_find_crlf(const char* str, int size);
char* irc_str_skip_to(char *str, char c);
int irc_parse_msg(char* buffer, irc_msg_t* msg, int lfpos);
int irc_process_msg(irc_session_t *s, irc_msg_t *m); //call the right even handler
int irc_handle_incoming_data(irc_session_t* s, int lfpos);
void irc_main_loop(irc_session_t* s);
void irc_send_join(irc_session_t* s, const char *channel);
void irc_send_privmsg(irc_session_t* s, const char *guy, const char *msg);
void irc_send_nick(irc_session_t* s, const char *nick);
void irc_sendv_privmsg(irc_session_t* s, const char *guy, const char *format, ...);
