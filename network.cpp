/*
 * TinyRobo
 */

#include "network.h"
#include <stdio.h>

#if defined(WIN32)
static WSADATA wsadata;
#endif

// for WIN32
int network_init()
{
#if defined(WIN32)
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif
    return 0;
}

int network_end()
{
#if defined(WIN32)
    WSACleanup();
#endif
    return 0;
}

SOCKET network_connect_IP(const char* ip, int port)
{
    if(!ip || port<=0) {
        printf("[!][network_connect_IP] Error: bad param!\n");
        return SOCKET_ERROR;
    }

    SOCKET sock = 0;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == SOCKET_ERROR) {
        printf("[!][network_connect_IP] Error: cant create socket!\n");
        return SOCKET_ERROR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    //
    // transformation of IP address from symbols into net-format
    //
    if (inet_addr(ip)!=INADDR_NONE) {
        addr.sin_addr.s_addr=inet_addr(ip);
    }
    else {
        //
        // try to get IP from domain name
        //

        HOSTENT *hst;
        if (hst=gethostbyname(ip)) {
            ((unsigned long *)&addr.sin_addr)[0]=
                    ((unsigned long **)hst->h_addr_list)[0][0];
        }
        else{
            printf("[!][network_connect_IP] Error: invalid IP: %s !\n", ip);
			network_close_connection(sock);
            return SOCKET_ERROR;
        }
    }

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[!][network_connect_IP] Error: cant connect: %s !\n", ip);
        return SOCKET_ERROR;
    }

    return sock;
}

void network_close_connection(SOCKET socket)
{
    if(socket == SOCKET_ERROR)
        return;

    SOCKET_CLOSE(socket);
}

SOCKET network_connect_UDP_IP(const char* ip, int port, struct sockaddr_in* addr)
{
    if(!ip || port<=0 || !addr) {
        printf("[!][network_connect_UDP_IP] Error: bad param!\n");
        return SOCKET_ERROR;
    }

    SOCKET sock = 0;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == SOCKET_ERROR) {
        printf("[!][network_connect_UDP_IP] Error: cant create socket!\n");
        return SOCKET_ERROR;
    }

    if (inet_addr(ip) == INADDR_NONE) {
        printf("[!][network_connect_UDP_IP] Error: invalid IP: %s !\n", ip);
		network_close_connection(sock);
        return SOCKET_ERROR;
    }

    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons( port );
    addr->sin_addr.s_addr = inet_addr( ip );

    return sock;
}

SOCKET network_create_TCP_server(int port, struct sockaddr_in* server)
{
    if(!server) {
        printf("[!][network_create_TCP_server] Error: bad param!\n"); //$LOG
        return SOCKET_ERROR;
    }

    SOCKET serversock;

    if ((serversock = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
        printf("[!][network_create_TCP_server] Error: socket() failed!\n"); //$LOG
        return SOCKET_ERROR;
    }

#if 1
    int optval = 1;
    if ( setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) ) {
        printf("[!][network_create_TCP_server] Error: Can't set SO_REUSEADDR on socket!\n");
		network_close_connection(serversock);
        return SOCKET_ERROR;
    }
#endif

    memset(server, 0, sizeof(*server));
    server->sin_family = AF_INET;			// adress family - Internet
    server->sin_port = htons( port );		// port
    server->sin_addr.s_addr = INADDR_ANY;	//

    if (bind(serversock, (const struct sockaddr *)server, sizeof(*server)) == -1) {
        printf("[!][network_create_TCP_server] Error: bind() to %d failed!\n", port); //$LOG
		network_close_connection(serversock);
        return SOCKET_ERROR;
    }

    // wait
    if (listen(serversock, SOMAXCONN) == -1) {
        printf("[!][network_create_TCP_server] Error: listen() failed!\n"); //$LOG
		network_close_connection(serversock);
        return SOCKET_ERROR;
    }

    return serversock;
}

SOCKET network_create_UDP_server(int port, struct sockaddr_in* server)
{
    if(!server) {
        printf("[!][network_create_UDP_server] Error: bad param!\n");
        return SOCKET_ERROR;
    }

    SOCKET serversock;

    if ((serversock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
        printf("[!][network_create_UDP_server] Error: socket() failed!\n");
        return SOCKET_ERROR;
    }

#if 1
    int optval = 1;
    if ( setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) ) {
        printf("[!][network_create_UDP_server] Error: Can't set SO_REUSEADDR on socket!\n");
		network_close_connection(serversock);
        return SOCKET_ERROR;
    }
#endif

    memset(server, 0, sizeof(*server));
    server->sin_family = AF_INET;               // adress family - Internet
    server->sin_port = htons( port );           // port
    server->sin_addr.s_addr = htonl(INADDR_ANY);//

    if (bind(serversock, (const struct sockaddr *)server, sizeof(*server)) == -1) {
        printf("[!][network_create_UDP_server] Error: bind() to %d failed!\n", port);
		network_close_connection(serversock);
        return SOCKET_ERROR;
    }

    return serversock;
}

#if defined(LINUX)
int network_create_UNIX_server(const char* path, struct sockaddr_un* server)
{
    if(!path || !server) {
        printf("[!][network_create_UNIX_server] Error: bad param!\n"); //$LOG
        return SOCKET_ERROR;
    }

    int sockfd=0, servlen=0;
    struct sockaddr_un  serv_addr;

    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == SOCKET_ERROR) {
        printf("[!][network_create_UNIX_server] Error: socket() failed!\n"); //$LOG
        return SOCKET_ERROR;
    }

    memset((char *) &serv_addr, 0, sizeof(struct sockaddr_un));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, path, sizeof(serv_addr.sun_path));

    servlen=strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    // remove socket file
    unlink(serv_addr.sun_path);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, servlen)<0) {
        printf("[!][network_create_UNIX_server] Error: bind() failed!\n"); //$LOG
		network_close_connection(sockfd);
        return SOCKET_ERROR;
    }

    if(listen(sockfd, SOMAXCONN) == -1) {
        printf("[!][network_create_UNIX_server] Error: listen() failed!\n"); //$LOG
		network_close_connection(sockfd);
        return SOCKET_ERROR;
    }

    return sockfd;
}

int network_connect_UNIX_socket(const char* path)
{
    if(!path) {
        printf("[!][network_connect_UNIX_socket] Error: bad param!\n"); //$LOG
        return SOCKET_ERROR;
    }
    int sock = SOCKET_ERROR;
    int servlen = 0;
    struct sockaddr_un  serv_addr;

    memset((char *)&serv_addr, 0, sizeof(struct sockaddr_un));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, path);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == SOCKET_ERROR) {
        printf("[!][network_connect_UNIX_socket] Error: socket() failed!\n");
        return SOCKET_ERROR;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, servlen) < 0) {
        printf("[!][network_connect_UNIX_socket] Error: connect() failed!\n");
		network_close_connection(sock);
        return SOCKET_ERROR;
    }

    return sock;
}
#endif //#if defined(LINUX)
