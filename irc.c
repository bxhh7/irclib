#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <memory.h>
#include <stdarg.h>
#include "irc.h"
#include "utils.h"
#include "net.h"

void die(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}


int irc_init_session(irc_session_t* s, const char* serv, const char* portno, const char* nick, const char* pass, irc_event_handler_set_t* es) {

	memset(s, 0, sizeof(irc_session_t));

	if (serv == NULL || portno == NULL || nick == NULL)
		return -1;
	s->server = strdup(serv);
	s->portno = strdup(portno);
	s->nick = strdup(nick);
	s->password = pass? strdup(pass) : NULL;
	s->buffer = (char *)malloc(IRC_MAX_BUFFER);
	s->pos = 0;
	s->event_handlers = *es;
	assert(s->buffer);

	s->connected = 0;
	return 0;
}
ssize_t irc_send_raw(irc_session_t* s, const char* format, ...) {
	char buffer[IRC_MAX_BUFFER];
	va_list ap;
	va_start(ap, format);

	int n = vsnprintf(buffer, IRC_MAX_BUFFER, format, ap);
	/* TODO: */
	va_end(ap);
	int nwrote = irc_socket_send(s->sockfd, buffer, n);
#ifdef IRC_DEBUG_MODE
	udebug("irc_send_raw(): wrote %d bytes into s->sockfd.", nwrote);
#endif
	return nwrote;
}


int irc_connect(irc_session_t* s) {
#ifdef IRC_DEBUG_MODE
	udebug("irc_connect(): connecting to %s:%s", s->server, s->portno);
#endif
	/* RFC 1459:
	 * The recommended order for a client to register is as follows:
	 * 1. Pass message
	 * 2. Nick message
	 * 3. User message */
	s->sockfd = irc_socket_create(s->server, s->portno);
	if (s->sockfd == -1) {
		return -1; // FIXME: proper error messages
	}
#ifdef IRC_DEBUG_MODE
	udebug("irc_connect() registering as NICK : %s USER: %s PASS: %s", s->nick, s->nick, s->password);
#endif
	if (s->password)
		irc_send_raw(s, "PASS %s \r\n", s->password);
	irc_send_raw(s, "NICK %s \r\n", s->nick);
	irc_send_raw(s, "USER %s * * : AAAA\r\n", s->nick);
#ifdef IRC_DEBUG_MODE
	udebug("irc_connect(), sucessfully connected.");
#endif
	/* some irc servers require that we ping them first */
	irc_send_raw(s, "PING : random\r\n");
	return 0;
}


char* irc_str_skip_to(char *str, char c) {
	while (*str) {
		if (*str == c)
			return str;
		str++;
	}
	return NULL;
}


// returns the first cr(lf) in the string
int irc_find_crlf(const char* str, int size) {
	for (int i = 0; i < size; i++) {
		if (str[i] == '\r' && str[i + 1] == '\n')
			return i + 1;
	}
	return -1;
}



// buffer must be null-terminated, complete, raw irc message(excluding lf)
int irc_parse_msg(char* buffer, irc_msg_t* msg, int lfpos) {
	/*
	 * From RFC 1459:
	 *  <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
	 *  <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
	 *  <command>  ::= <letter> { <letter> } | <number> <number> <number>
	 *  <SPACE>    ::= ' ' { ' ' }
	 *  <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
	 *  <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
	 *                 or NUL or CR or LF, the first of which may not be ':'>
	 *  <trailing> ::= <Any, possibly *empty*, sequence of octets not including
	 *                   NUL or CR or LF>
	 */
	memset(msg, 0, sizeof(irc_msg_t));

	char *p = buffer;
	char *prefix;
	if (*p == ':') {
		char *prefix_end = irc_str_skip_to(p, ' ');
		if (!prefix_end) {
#ifdef IRC_DEBUG_MODE
			uerror("invalid message, parsing the prefix");
#endif
			goto invalid_msg;
		}
		*prefix_end = '\0';
		prefix = p + 1;

		msg->prefix_name = prefix;
		char *n = irc_str_skip_to(prefix, '!');
		if (n < prefix_end && n != NULL) {
			// there's ['!' <user>] in the prefix.
			*n = '\0';
			msg->prefix_user = n + 1;
		}

		n = irc_str_skip_to(prefix, '@');
		if (n < prefix_end && n != NULL) {
			*n = '\0';
			msg->prefix_host = n + 1;
		}

		*prefix_end = '\0';
		p = prefix_end + 1;
#ifdef IRC_DEBUG_MODE
		udebug("msg->prefix_name = %s, msg->prefix_user = %s, msg->prefix_host = %s", msg->prefix_name, msg->prefix_user, msg->prefix_host);
#endif
	}

	char *params_start = irc_str_skip_to(p, ' ');

	if (!params_start) {
#ifdef IRC_DEBUG_MODE
		uerror("we don't have params_start.");
#endif
		goto invalid_msg;
	}

	*params_start = '\0';
	msg->cmd = p;


	p = params_start + 1;
	char *params_end = irc_str_skip_to(p, ':');
	if (!params_end) {
#ifdef IRC_DEBUG_MODE
		uwarn("we don't have trailing_start");
#endif
		params_end = buffer + lfpos; // apparently some servers don't have <trailing> with every msg
	}
	char *s = p;
	int parameter_index = 0;
	while (p != params_end && parameter_index < IRC_MAX_PARAMS - 1) {
		if (*p == ' ') {
			*p = '\0';
			msg->parameters[parameter_index++] = s;
			s = p + 1;
		}
		p++;
	}
	if(params_end != buffer + lfpos) { // if we had ':'
		msg->parameters[parameter_index++] = params_end + 1; // it's already '\0' at the end
	} else { // if we didn't have ':'
		msg->parameters[parameter_index++] = s;
	}
	msg->parameters_size = parameter_index;

#ifdef IRC_DEBUG_MODE
	udebug("irc_parse_msg(); msg->cmd='%s'",  msg->cmd);

	if (msg->parameters_size) {
		udebug("irc_parse_msg(); msg->params are:");
		for (int i = 0; i < msg->parameters_size; i++) {
			printf("	[%d]: '%s'\n", i, msg->parameters[i]);
		}
	}
#endif

	return 0;
invalid_msg:


#ifdef IRC_DEBUG_MODE
	uerror("invalid message(%s)", buffer);
#endif


	return -1;
}

int irc_process_msg(irc_session_t *s, irc_msg_t *m) { /* call the right event handler, this is where you can handle more of the irc protocol */
	int32_t cmd = WORDL(m->cmd[0], m->cmd[1], m->cmd[2], m->cmd[3]);
	int ret = 0;


	udebug("recived an even?? %s", m->cmd);
	/* FIXME: check m->parameters_size for validity of messages, if it doesnt have a sufficent number of
	 * arguemtns, don't pass it to the handlers */
	switch(cmd) {
		case WORDL('P', 'I', 'N', 'G'):

			/* apparently some IRC servers require PONG :<param> and some require PONG <param>,
			 * and some require both! */
			if (s->event_handlers.ping)
				ret = s->event_handlers.ping(s, m->parameters[0]);
			if (m->parameters_size > 1)
				irc_send_raw(s, "PONG %s :%s\r\n", m->parameters[0], m->parameters[1]);
			else
				irc_send_raw(s, "PONG :%s\r\n", m->parameters[0]);
			break;


		case WORDL('P', 'R', 'I', 'V'):
			if (s->event_handlers.privmsg) 
				ret = s->event_handlers.privmsg(s, 
						(m->parameters[0][0] == '#' ? m->parameters[0] : m->prefix_name),
						m->parameters[1]);
			break;


		case WORDL('I', 'N', 'V', 'I'):
			if (s->event_handlers.invite)
				ret = s->event_handlers.invite(s, m->parameters[1]);
			break;
		
		default: 
			if (s->event_handlers.default_handler) 
				ret = s->event_handlers.default_handler(s, m);
			break;
	}
	return ret;
}
int irc_handle_incoming_data(irc_session_t* s, int lfpos) {

	irc_msg_t msg;

	s->buffer[lfpos - 1] = '\0';
	if (irc_parse_msg(s->buffer, &msg, lfpos) == -1) {
#ifdef IRC_DEBUG_MODE
		uerror("irc_handle_incoming_adta(): invalid server command. \n");
#endif
		return -1;
	}

	int r = irc_process_msg(s, &msg); // call the right callbacks.
	if (r != 0) {
#ifdef IRC_DEBUG_MODE
		uerror("irc_process_msg() wasn't sucessfull, ignoring the server message.");
#endif
		return -1;
	}

	return 0;
}


void irc_main_loop(irc_session_t* s) {
	while (1) {	
#ifdef IRC_DEBUG_MODE
		udebug("irc_main_loop();");
#endif
		int nread = irc_socket_recv(s->sockfd, s->buffer + s->pos, IRC_MAX_BUFFER - s->pos);
		if (nread <= 0) {
#ifdef IRC_DEBUG_MODE
			uerror("irc_socket_recv() problem in irc_main_loop(), exiting for now.\n");
			// FIXME proper error messages
#endif
			exit(EXIT_FAILURE);
		}

		s->pos += nread;

		int lfpos;
		while ((lfpos = irc_find_crlf(s->buffer,s->pos)) != -1) {
			irc_handle_incoming_data(s, lfpos);
			memmove(s->buffer, s->buffer + lfpos + 1, s->pos - lfpos);
			s->pos = s->pos - (lfpos + 1);
		}

	}
}


void irc_send_join(irc_session_t* s, const char *channel) {
	irc_send_raw(s, "JOIN %s\r\n", channel);
}

void irc_send_privmsg(irc_session_t* s, const char *guy, const char *msg) {
	irc_send_raw(s, "PRIVMSG %s :%s\r\n", guy, msg);
}

void irc_send_nick(irc_session_t* s, const char *nick) {
	irc_send_raw(s, "NICK %s : \r\n", s, nick);
}

void irc_sendv_privmsg(irc_session_t* s, const char *guy, const char *format, ...) {
	char buffer[IRC_MAX_MSG]; // FIXME 
	va_list ap;

	va_start(ap, format);

	vsnprintf(buffer, sizeof(buffer), format, ap);
	irc_send_privmsg(s, guy, buffer);

	va_end(ap);
}
