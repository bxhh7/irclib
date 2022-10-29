#include <stdio.h>
#include <string.h>
#include "irc.h"
#include "utils.h"

int invite_handler(irc_session_t* s, const char* chan) {
	udebug("got invited to channel %s, joining the channel.", chan); 
	
	irc_send_join(s, chan);
	return 0;
}

int privmsg_handler(irc_session_t* s, const char* origin, const char* text) {
	udebug("privmsg_handler(): message from %s : %s", origin, text);
	
	irc_send_privmsg(s, origin, text);
	return 0;
}

int ping_handler(irc_session_t* s, const char* param) {
	// libirc handles PINGs itself, but we can use this handler for debuugin purposes maybe
	udebug("PING %s", param);
	return 0;
}

int main() {
	irc_session_t sess;
	irc_event_handler_set_t es;

	memset(&es, 0, sizeof(es)); /* it is very important that all the unused event handlers are set to NULL */
	es.invite = invite_handler;
	es.privmsg = privmsg_handler;
	es.ping = ping_handler;

	irc_init_session(&sess, /* session */
			"bsdforall.org",  /* server */
			"6667", /* port */
			"mister_echo", /* nick name */
			NULL,  /* password */
			&es); /* event handlers */
	if (irc_connect(&sess)) {
		uerror("irc_connect() failed.");
		return -1;
	}

	irc_main_loop(&sess); /* this blocks, libirc doesnt suppport multiple
						  sessions, however, nothing's stopping you 
						  from applying your own threading 
						  mechanism to it. libirc has no global state and thus, you
						  can have DIFFERENT IRC SESSIONS in different threads without 
						  any syncrhonization.*/
}

