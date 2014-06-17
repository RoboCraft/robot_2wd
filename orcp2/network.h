/*
 * TinyRobo
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"


#if defined(WIN32)
//# include <windows.h>
# if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#  include <winsock2.h>
#endif

# ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
# endif

#elif defined(LINUX)
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>

# include <sys/ioctl.h>
# include <sys/select.h>

# include <sys/un.h>

typedef int SOCKET;
typedef struct hostent HOSTENT;
//# define SOCKET int
//# define HOSTENT struct hostent

# ifndef SOCKET_ERROR
#  define INVALID_SOCKET  (SOCKET)(~0)
#  define SOCKET_ERROR            (-1)
# endif

#endif //#ifdef WIN32

#if defined(WIN32)
# define SOCKET_CLOSE(x) shutdown(x, 2); closesocket(x);
#elif defined(LINUX)
# define SOCKET_CLOSE(x) shutdown(x, SHUT_RDWR); close(x);
#endif //#ifdef WIN32

// for WIN32
int network_init();
int network_end();

SOCKET network_connect_IP(const char* ip, int port);
void network_close_connection(SOCKET socket);

SOCKET network_connect_UDP_IP(const char* ip, int port, struct sockaddr_in* addr);
SOCKET network_create_TCP_server(int port, struct sockaddr_in* server);
SOCKET network_create_UDP_server(int port, struct sockaddr_in* server);

#if defined(LINUX)
int network_create_UNIX_server(const char* path, struct sockaddr_un* server);
int network_connect_UNIX_socket(const char* path);
#endif //#ifdef LINUX


#endif /* #ifndef NETWORK_H */
