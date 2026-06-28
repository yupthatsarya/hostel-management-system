#ifndef SERVER_H
#define SERVER_H

#include "common.h"

/* Starts the C-based HTTP web server on the given port.
   Handles connections, routes static assets or JSON APIs, and returns when shutdown. */
void startWebServer(int port);

#endif /* SERVER_H */
