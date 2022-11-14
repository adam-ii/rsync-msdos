#include <sys/socket.h>
#include <time.h>

int W32_CALL ioctlsocket (int s, long cmd, char *argp)
{
    return 0;
}

int W32_CALL bind (int s, const struct sockaddr *myaddr, socklen_t namelen)
{
    return 0;
}

int W32_CALL recv (int s, void *buf, int len, int flags)
{
    return 0;
}

int W32_CALL send (int s, const void *buf, int len, int flags)
{
    return 0;
}

int W32_CALL select_s (int nfds, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout)
{
    return 0;
}

int W32_CALL socket (int family, int type, int protocol)
{
    return 0;
}

int W32_CALL connect (int s, const struct sockaddr *servaddr, socklen_t addrlen)
{
    return 0;
}

int W32_CALL getnameinfo (const struct sockaddr *sa, socklen_t salen,
                          char *host, socklen_t hostlen,
                          char *serv, socklen_t servlen, int flags)
{
    return 0;
}

struct hostent * W32_CALL gethostbyaddr (const char *addr_name, int len, int type)
{
    return NULL;
}

struct hostent * W32_CALL gethostbyname (const char *name)
{
    return NULL;
}

struct servent * W32_CALL getservbyname (const char *serv, const char *proto)
{
    return NULL;
}

int W32_CALL getpeername (int s, struct sockaddr *name, socklen_t *namelen)
{
    return 0;
}

int h_errno;
