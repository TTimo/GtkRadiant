
#if !defined(INCLUDED_SOCKETS_H)
#define INCLUDED_SOCKETS_H

#include "l_net/l_net.h"

// waits for a socket to become ready
// returns
// -1: error
// 0: timeout
// 1: ready
int Net_Wait(socket_t *sock, long sec, long usec);

#endif
