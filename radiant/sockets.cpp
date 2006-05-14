
#include "sockets.h"

#if defined(WIN32)
#include <winsock2.h>
#elif defined (POSIX)
#include <sys/time.h>
#define SOCKET_ERROR -1
#else
#error "unsupported platform"
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

int Net_Wait(socket_t *sock, long sec, long usec)
{
// used for select()
#ifdef WIN32
  TIMEVAL tout = { sec, usec };
#endif
#if defined (POSIX)
	timeval tout;
	tout.tv_sec = sec;
	tout.tv_usec = usec;
#endif

  // select() will identify if the socket needs an update
  // if the socket is identified that means there's either a message or the connection has been closed/reset/terminated
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(((unsigned int)sock->socket), &readfds);
	// from select man page:
	// n is the highest-numbered descriptor in any of the three sets, plus 1
	// (no use on windows)
  switch( select( sock->socket + 1, &readfds, 0, 0, &tout ) )
  {
  case SOCKET_ERROR:
    return -1;
  case 0:
    return 0;
  default:
    return 1;
  }
}

