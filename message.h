#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "common.h"

#define MSG_NOMESSAGE	-1

#define MSG_REQUEST	-2
#define MSG_CONNECT	-3

 /* prototypes */

boolean msg_is_connected (void);
boolean msg_open_connection (char *hostname, char *portname);
void msg_close_connection (void);

int msg_recv (void *optional, int length);
int msg_send (int msg, void *optional, int length);
boolean msg_arrived (void);

#endif
